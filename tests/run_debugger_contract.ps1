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
