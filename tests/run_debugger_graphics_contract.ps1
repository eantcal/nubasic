# run_debugger_graphics_contract.ps1 - contract test for graphical debug mode
#
# Verifies that nubasicdebug can run with both the machine interface and a
# separate GDI graphics window. The debugger protocol must remain on stdout,
# while graphical statements execute without taking over the debug channel.

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
        "$RepoRoot\build\debug\cli\win\Release\nubasicdebug.exe"
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return (Resolve-Path -Path $candidate).Path
        }
    }

    throw "nubasicdebug not found. Build the debugger backend first or pass -Interpreter <path>."
}

$Interpreter = Resolve-DebugInterpreter -RepoRoot $RepoRoot -Interpreter $Interpreter
Write-Host "Graphics debugger interpreter: $Interpreter"
Write-Host "Graphics debugger test dir    : $TestDir"

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $Interpreter
$psi.WorkingDirectory = $TestDir
$psi.UseShellExecute = $false
$psi.RedirectStandardInput = $true
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
$psi.CreateNoWindow = $true
$psi.Arguments = "--machine-interface --graphics-window"

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
        [int]$InterruptAfterMs = 300,
        [int]$TimeoutMs = 7000
    )

    $before = (Get-OutputSnapshot).stdout.Length
    Write-Host ">>> $Command"
    $proc.StandardInput.WriteLine($Command)
    $proc.StandardInput.Flush()

    Start-Sleep -Milliseconds $InterruptAfterMs

    Write-Host ">>> <ETX>"
    $proc.StandardInput.Write([char]3)
    $proc.StandardInput.Flush()

    Wait-Until -Description "graphics debug pause stop" -TimeoutMs $TimeoutMs -Condition {
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
        throw "Failed to start graphics debugger process."
    }

    $proc.StandardInput.AutoFlush = $true
    $proc.BeginOutputReadLine()
    $proc.BeginErrorReadLine()

    Wait-ForPattern -Pattern '@@nubasic event="ready"' -Description "machine-interface ready event"

    $startup = Get-OutputSnapshot
    if ($startup.stderr -match 'Failed to initialize GDI graphics window') {
        throw "GDI graphics window failed to initialize.`nSTDERR:`n$($startup.stderr)"
    }

    $loadOutput = Send-Command -Command 'load "test_debugger_graphics.bas"' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "LOAD completion"
    if ($loadOutput -notmatch '@@nubasic event="ok"') {
        throw "LOAD did not complete successfully.`nOutput:`n$loadOutput"
    }

    $null = Send-Command -Command 'clrbrk' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "ClrBrK completion"

    $null = Send-Command -Command 'break 5' `
        -CompletionPattern '@@nubasic event="ok"' `
        -Description "graphics breakpoint creation"

    $runOutput = Send-Command -Command 'run' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "RUN graphics breakpoint stop"
    if ($runOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="breakpoint"[^\r\n]*line="5"') {
        throw "RUN did not stop on graphical breakpoint line 5.`nOutput:`n$runOutput"
    }
    if ($runOutput -notmatch '@@nubasic event="stopped"[^\r\n]*sourceLine="5"[^\r\n]*source="[^"]*test_debugger_graphics\.bas"') {
        throw "Graphics breakpoint stop did not include source metadata.`nOutput:`n$runOutput"
    }

    $stepOutput = Send-Command -Command 'stepover' `
        -CompletionPattern '@@nubasic event="stopped"' `
        -Description "graphics stepover stop"
    if ($stepOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="step"[^\r\n]*line="6"') {
        throw "StepOver did not advance to line 6 after drawing line.`nOutput:`n$stepOutput"
    }

    $pauseOutput = Send-Command-And-Interrupt -Command 'cont'
    if ($pauseOutput -notmatch '@@nubasic event="stopped"[^\r\n]*reason="pause"[^\r\n]*line="\d+"') {
        throw "Graphics debug pause did not report a stopped pause event.`nOutput:`n$pauseOutput"
    }
    if ($pauseOutput -match '@@nubasic event="terminated"') {
        throw "Graphics debug pause terminated the program instead of suspending it.`nOutput:`n$pauseOutput"
    }

    Write-Host ""
    Write-Host "PASS graphics debugger backend contract"
    exit 0
}
catch {
    Write-Host ""
    Write-Host "FAIL graphics debugger backend contract"
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

    Remove-Job -State Completed -ErrorAction SilentlyContinue
}
