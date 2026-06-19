# run_debugger_contract.ps1 - contract test for nubasicdebug machine interface
#
# Verifies the debugger backend contract expected by the VS Code extension:
# - machine-interface startup event
# - command completion for load/clrbrk/break/ston/stoff/vars
# - breakpoint stop on RUN
# - single-step stop on CONT while step mode is active
#
# Exit code: 0 = pass, 1 = failure.

param(
    [string]$Interpreter = "",
    [string]$TestDir = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent $ScriptDir

if ($TestDir -eq "") {
    $TestDir = $ScriptDir
}
$TestDir = (Resolve-Path -Path $TestDir).Path

function Resolve-DebugInterpreter {
    param([string]$RepoRoot, [string]$Interpreter)

    if ($Interpreter -and (Test-Path $Interpreter)) {
        return (Resolve-Path -Path $Interpreter).Path
    }

    $candidates = @(
        "$RepoRoot\_generated\build\raycast-release\cli\win\Release\nubasicdebug.exe",
        "$RepoRoot\_generated\build\raycast-release\cli\win\Debug\nubasicdebug.exe",
        "$RepoRoot\_generated\build\release\cli\win\Release\nubasicdebug.exe",
        "$RepoRoot\_generated\build\release\cli\win\Debug\nubasicdebug.exe",
        "$RepoRoot\_generated\build\debug\cli\win\Debug\nubasicdebug.exe",
        "$RepoRoot\_generated\build\debug\cli\win\Release\nubasicdebug.exe",
        "$RepoRoot\_generated\build\release\nubasicdebug",
        "$RepoRoot\_generated\build\debug\nubasicdebug",
        "$RepoRoot\build\release\cli\win\Release\nubasicdebug.exe",
        "$RepoRoot\build\release\cli\win\Debug\nubasicdebug.exe",
        "$RepoRoot\build\debug\cli\win\Debug\nubasicdebug.exe",
        "$RepoRoot\build\debug\cli\win\Release\nubasicdebug.exe",
        "$RepoRoot\build\release\nubasicdebug",
        "$RepoRoot\build\debug\nubasicdebug"
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return (Resolve-Path -Path $candidate).Path
        }
    }

    throw "nubasicdebug not found. Build the debugger backend first or pass -Interpreter <path>."
}

function Escape-Regex {
    param([string]$Text)
    return [Regex]::Escape($Text)
}

$Interpreter = Resolve-DebugInterpreter -RepoRoot $RepoRoot -Interpreter $Interpreter
Write-Host "Debugger interpreter: $Interpreter"
Write-Host "Debugger test dir    : $TestDir"

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $Interpreter
$psi.WorkingDirectory = $TestDir
$psi.UseShellExecute = $false
$psi.RedirectStandardInput = $true
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
$psi.CreateNoWindow = $true
$psi.Arguments = "--machine-interface"

$proc = New-Object System.Diagnostics.Process
$proc.StartInfo = $psi

$stdout = New-Object System.Text.StringBuilder
$stderr = New-Object System.Text.StringBuilder
$syncRoot = New-Object object

$null = Register-ObjectEvent -InputObject $proc -EventName OutputDataReceived -Action {
    if ($EventArgs.Data -ne $null) {
        [System.Threading.Monitor]::Enter($syncRoot)
        try {
            [void]$stdout.AppendLine($EventArgs.Data)
        } finally {
            [System.Threading.Monitor]::Exit($syncRoot)
        }
    }
}

$null = Register-ObjectEvent -InputObject $proc -EventName ErrorDataReceived -Action {
    if ($EventArgs.Data -ne $null) {
        [System.Threading.Monitor]::Enter($syncRoot)
        try {
            [void]$stderr.AppendLine($EventArgs.Data)
        } finally {
            [System.Threading.Monitor]::Exit($syncRoot)
        }
    }
}

function Get-OutputSnapshot {
    [System.Threading.Monitor]::Enter($syncRoot)
    try {
        return @{
            stdout = $stdout.ToString()
            stderr = $stderr.ToString()
        }
    } finally {
        [System.Threading.Monitor]::Exit($syncRoot)
    }
}

function Wait-Until {
    param(
        [scriptblock]$Condition,
        [string]$Description,
        [int]$TimeoutMs = 5000
    )

    $deadline = [DateTime]::UtcNow.AddMilliseconds($TimeoutMs)
    while ([DateTime]::UtcNow -lt $deadline) {
        if (& $Condition) {
            return
        }
        Start-Sleep -Milliseconds 25
    }

    $snapshot = Get-OutputSnapshot
    throw "Timeout waiting for $Description.`nSTDOUT:`n$($snapshot.stdout)`nSTDERR:`n$($snapshot.stderr)"
}

function Wait-ForPattern {
    param(
        [string]$Pattern,
        [string]$Description,
        [int]$TimeoutMs = 5000
    )

    Wait-Until -Description $Description -TimeoutMs $TimeoutMs -Condition {
        $snapshot = Get-OutputSnapshot
        return $snapshot.stdout -match $Pattern
    }
}

function Send-Command {
    param(
        [string]$Command,
        [string]$CompletionPattern,
        [string]$Description
    )

    $before = (Get-OutputSnapshot).stdout.Length
    Write-Host ">>> $Command"
    $proc.StandardInput.WriteLine($Command)
    $proc.StandardInput.Flush()

    Wait-Until -Description $Description -TimeoutMs 5000 -Condition {
        $snapshot = Get-OutputSnapshot
        if ($snapshot.stdout.Length -lt $before) {
            return $false
        }
        $chunk = $snapshot.stdout.Substring($before)
        return $chunk -match $CompletionPattern
    }

    $snapshot = Get-OutputSnapshot
    return $snapshot.stdout.Substring($before)
}

function Send-Command-And-Interrupt {
    param(
        [string]$Command,
        [int]$InterruptAfterMs = 250,
        [int]$TimeoutMs = 5000
    )

    $before = (Get-OutputSnapshot).stdout.Length
    Write-Host ">>> $Command"
    $proc.StandardInput.WriteLine($Command)
    $proc.StandardInput.Flush()

    Start-Sleep -Milliseconds $InterruptAfterMs

    Write-Host ">>> <ETX>"
    $proc.StandardInput.Write([char]3)
    $proc.StandardInput.Flush()

    Wait-Until -Description "debug pause stop" -TimeoutMs $TimeoutMs -Condition {
        $snapshot = Get-OutputSnapshot
        if ($snapshot.stdout.Length -lt $before) {
            return $false
        }
        $chunk = $snapshot.stdout.Substring($before)
        return $chunk -match '@@nubasic event="(?:stopped|interrupted|terminated)"'
    }

    $snapshot = Get-OutputSnapshot
    return $snapshot.stdout.Substring($before)
}

try {
    if (-not $proc.Start()) {
        throw "Failed to start debugger process."
    }

    $proc.StandardInput.AutoFlush = $true
    $proc.BeginOutputReadLine()
    $proc.BeginErrorReadLine()

    Wait-ForPattern -Pattern '@@nubasic event="ready"' -Description "machine-interface ready event"

    $loadOutput = Send-Command -Command 'load "test_debugger_contract.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion"
    if ($loadOutput -notmatch '@@nubasic event="ok"') {
        throw "LOAD did not complete successfully."
    }

    $stepLoadOutput = Send-Command -Command 'load "test_debugger_step_entry.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion for entry-step contract"
    if ($stepLoadOutput -notmatch '@@nubasic event="ok"') {
        throw "Entry-step LOAD did not complete successfully."
    }

    $null = Send-Command -Command 'clrbrk' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "ClrBrk completion before entry-step contract"

    $entryStepOutput = Send-Command -Command 'step' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "entry-step stop on first executable line"
    if ($entryStepOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="step"[^\r\n]*line="2"') {
        throw "Entry-step did not stop on line 2 before executing the first statement.`nOutput:`n$entryStepOutput"
    }
    if ($entryStepOutput -notmatch '@@nubasic event="stopped"[^\r\n]*sourceLine="2"[^\r\n]*source="[^"]*test_debugger_step_entry\.bas"') {
        throw "Entry-step stop did not include source location metadata.`nOutput:`n$entryStepOutput"
    }

    $stepIntoLoadOutput = Send-Command -Command 'load "test_debugger_step_entry.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion for stepinto alias contract"
    if ($stepIntoLoadOutput -notmatch '@@nubasic event="ok"') {
        throw "StepInto LOAD did not complete successfully."
    }

    $null = Send-Command -Command 'clrbrk' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "ClrBrk completion before stepinto alias contract"

    $stepIntoOutput = Send-Command -Command 'stepinto' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "stepinto alias stop on first executable line"
    if ($stepIntoOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="step"[^\r\n]*line="2"') {
        throw "StepInto alias did not stop on line 2 before executing the first statement.`nOutput:`n$stepIntoOutput"
    }

    $loadOutput = Send-Command -Command 'load "test_debugger_contract.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion after entry-step contract"
    if ($loadOutput -notmatch '@@nubasic event="ok"') {
        throw "LOAD did not complete successfully after entry-step contract."
    }

    $null = Send-Command -Command 'clrbrk' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "ClrBrk completion"

    $null = Send-Command -Command 'break 3' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "breakpoint creation"

    $runOutput = Send-Command -Command 'run' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "RUN breakpoint stop"
    if ($runOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="breakpoint"[^\r\n]*line="3"') {
        throw "RUN did not stop on breakpoint line 3.`nOutput:`n$runOutput"
    }
    if ($runOutput -notmatch '@@nubasic event="stopped"[^\r\n]*sourceLine="3"[^\r\n]*source="[^"]*test_debugger_contract\.bas"') {
        throw "RUN breakpoint stop did not include source location metadata.`nOutput:`n$runOutput"
    }

    $varsOutput = Send-Command -Command 'vars' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "VARS completion after breakpoint"
    if ($varsOutput -notmatch 'Current line\s*:\s*3' -and $varsOutput -notmatch '(?m)^\s*3\s+Print "beta"' ) {
        throw "Debugger state did not identify line 3 after breakpoint.`nOutput:`n$varsOutput"
    }

    $null = Send-Command -Command 'ston' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "step mode enable"

    $contOutput = Send-Command -Command 'cont' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "single-step stop"
    if ($contOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="step"[^\r\n]*line="4"') {
        throw "Single-step did not stop on line 4.`nOutput:`n$contOutput"
    }
    if ($contOutput -notmatch '@@nubasic event="stopped"[^\r\n]*sourceLine="4"[^\r\n]*source="[^"]*test_debugger_contract\.bas"') {
        throw "Single-step stop did not include source location metadata.`nOutput:`n$contOutput"
    }

    $null = Send-Command -Command 'stoff' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "step mode disable"

    $stepModesLoadOutput = Send-Command -Command 'load "test_debugger_step_modes.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion for step mode contracts"
    if ($stepModesLoadOutput -notmatch '@@nubasic event="ok"') {
        throw "Step mode LOAD did not complete successfully."
    }

    $null = Send-Command -Command 'clrbrk' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "ClrBrk completion before stepover contract"

    $null = Send-Command -Command 'break 3' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "stepover breakpoint creation"

    $stepOverRunOutput = Send-Command -Command 'run' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "stepover RUN breakpoint stop"
    if ($stepOverRunOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="breakpoint"[^\r\n]*line="3"') {
        throw "StepOver setup did not stop on line 3.`nOutput:`n$stepOverRunOutput"
    }

    $stepOverOutput = Send-Command -Command 'stepover' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "stepover stop after procedure call"
    if ($stepOverOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="step"[^\r\n]*line="4"') {
        throw "StepOver did not stop on line 4 after the procedure call.`nOutput:`n$stepOverOutput"
    }

    $stepOutLoadOutput = Send-Command -Command 'load "test_debugger_step_modes.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion for stepout contract"
    if ($stepOutLoadOutput -notmatch '@@nubasic event="ok"') {
        throw "StepOut LOAD did not complete successfully."
    }

    $null = Send-Command -Command 'clrbrk' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "ClrBrk completion before stepout contract"

    $null = Send-Command -Command 'break 8' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "stepout breakpoint creation"

    $stepOutRunOutput = Send-Command -Command 'run' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "stepout RUN breakpoint stop"
    if ($stepOutRunOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="breakpoint"[^\r\n]*line="8"') {
        throw "StepOut setup did not stop inside Foo on line 8.`nOutput:`n$stepOutRunOutput"
    }

    $stepOutOutput = Send-Command -Command 'stepout' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "stepout stop in caller"
    if ($stepOutOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="step"[^\r\n]*line="4"') {
        throw "StepOut did not stop on line 4 in the caller.`nOutput:`n$stepOutOutput"
    }

    $nestedSubLoadOutput = Send-Command -Command 'load "test_debug_nested_sub_continue.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion for nested sub continue contract"
    if ($nestedSubLoadOutput -notmatch '@@nubasic event="ok"') {
        throw "Nested sub continue LOAD did not complete successfully."
    }

    $null = Send-Command -Command 'clrbrk' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "ClrBrk completion before nested sub continue contract"

    $null = Send-Command -Command 'break 14' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "nested sub breakpoint creation"

    $nestedSubRunOutput = Send-Command -Command 'run' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "nested sub RUN breakpoint stop"
    if ($nestedSubRunOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="breakpoint"[^\r\n]*line="14"') {
        throw "Nested sub setup did not stop on line 14.`nOutput:`n$nestedSubRunOutput"
    }

    $nestedSubContOutput = Send-Command -Command 'cont' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "nested sub continue completion"
    if ($nestedSubContOutput -match '@@nubasic event="runtimeError"') {
        throw "Nested sub continue raised a runtime error.`nOutput:`n$nestedSubContOutput"
    }
    if ($nestedSubContOutput -notmatch 'lifeState=\s*2\s+killed=\s*1') {
        throw "Nested sub continue did not return to Main with locals intact.`nOutput:`n$nestedSubContOutput"
    }

    $stepExprLoadOutput = Send-Command -Command 'load "test_debugger_step_expr.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion for expression step contracts"
    if ($stepExprLoadOutput -notmatch '@@nubasic event="ok"') {
        throw "Expression step LOAD did not complete successfully."
    }

    $null = Send-Command -Command 'clrbrk' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "ClrBrk completion before expression stepover contract"

    $null = Send-Command -Command 'break 3' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "expression stepover breakpoint creation"

    $stepExprRunOutput = Send-Command -Command 'run' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "expression stepover RUN breakpoint stop"
    if ($stepExprRunOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="breakpoint"[^\r\n]*line="3"') {
        throw "Expression StepOver setup did not stop on line 3.`nOutput:`n$stepExprRunOutput"
    }

    $stepExprOverOutput = Send-Command -Command 'stepover' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "expression stepover stop after function call"
    if ($stepExprOverOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="step"[^\r\n]*line="4"') {
        throw "Expression StepOver did not stop on line 4 after the function call.`nOutput:`n$stepExprOverOutput"
    }
    if ($stepExprOverOutput -match '@@nubasic event="stopped"[^\r\n]*line="8"') {
        throw "Expression StepOver entered the function body like StepInto.`nOutput:`n$stepExprOverOutput"
    }

    $stepExprOutLoadOutput = Send-Command -Command 'load "test_debugger_step_expr.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion for expression stepout contract"
    if ($stepExprOutLoadOutput -notmatch '@@nubasic event="ok"') {
        throw "Expression stepout LOAD did not complete successfully."
    }

    $null = Send-Command -Command 'clrbrk' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "ClrBrk completion before expression stepout contract"

    $null = Send-Command -Command 'break 8' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "expression stepout breakpoint creation"

    $stepExprOutRunOutput = Send-Command -Command 'run' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "expression stepout RUN breakpoint stop"
    if ($stepExprOutRunOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="breakpoint"[^\r\n]*line="8"') {
        throw "Expression StepOut setup did not stop inside the function at line 8.`nOutput:`n$stepExprOutRunOutput"
    }

    $stepExprOutOutput = Send-Command -Command 'stepout' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "expression stepout stop after returning to caller"
    if ($stepExprOutOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="step"[^\r\n]*line="4"') {
        throw "Expression StepOut did not stop on line 4 after returning to the caller.`nOutput:`n$stepExprOutOutput"
    }
    if ($stepExprOutOutput -match '@@nubasic event="stopped"[^\r\n]*line="10"') {
        throw "Expression StepOut stopped on the function boundary instead of the caller.`nOutput:`n$stepExprOutOutput"
    }

    $stepOutInputLoadOutput = Send-Command -Command 'load "test_debugger_stepout_input.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion for stepout input contract"
    if ($stepOutInputLoadOutput -notmatch '@@nubasic event="ok"') {
        throw "StepOut input LOAD did not complete successfully."
    }

    $null = Send-Command -Command 'clrbrk' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "ClrBrk completion before stepout input contract"

    $null = Send-Command -Command 'break 9' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "stepout input breakpoint creation"

    $stepOutInputRunOutput = Send-Command -Command 'run' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "stepout input RUN breakpoint stop"
    if ($stepOutInputRunOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="breakpoint"[^\r\n]*line="9"') {
        throw "StepOut input setup did not stop before the input line.`nOutput:`n$stepOutInputRunOutput"
    }

    $stepOutInputOutput = Send-Command -Command 'stepout' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "stepout stops before blocking input"
    if ($stepOutInputOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="step"[^\r\n]*line="(?:8|9)"') {
        throw "StepOut did not stop at or immediately after the blocking Input$ line.`nOutput:`n$stepOutInputOutput"
    }

    $pauseLoadOutput = Send-Command -Command 'load "test_debugger_pause.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion for pause contract"
    if ($pauseLoadOutput -notmatch '@@nubasic event="ok"') {
        throw "Pause LOAD did not complete successfully."
    }

    $null = Send-Command -Command 'clrbrk' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "ClrBrk completion before pause contract"

    $pauseEntryOutput = Send-Command -Command 'stepinto' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "pause contract stop on entry"
    if ($pauseEntryOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="step"[^\r\n]*line="2"') {
        throw "Pause setup did not stop on entry line 2.`nOutput:`n$pauseEntryOutput"
    }

    $pauseOutput = Send-Command-And-Interrupt -Command 'cont'
    if ($pauseOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="pause"[^\r\n]*line="\d+"') {
        throw "Debug pause did not report a stopped pause event.`nOutput:`n$pauseOutput"
    }
    if ($pauseOutput -match '@@nubasic event="terminated"') {
        throw "Debug pause terminated the program instead of suspending it.`nOutput:`n$pauseOutput"
    }

    Write-Host ""
    Write-Host "PASS debugger backend contract"
    exit 0
}
catch {
    Write-Host ""
    Write-Host "FAIL debugger backend contract"
    Write-Host ($_ | Out-String)
    exit 1
}
finally {
    try {
        if ($proc -and -not $proc.HasExited) {
            $proc.StandardInput.WriteLine("exit")
            if (-not $proc.WaitForExit(1000)) {
                $proc.Kill()
                $proc.WaitForExit()
            }
        }
    }
    catch {
    }

    Get-EventSubscriber | Where-Object {
        $_.SourceObject -eq $proc
    } | Unregister-Event -Force -ErrorAction SilentlyContinue

    if ($proc) {
        $proc.Dispose()
    }
}
