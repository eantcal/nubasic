param(
    [string]$Interpreter = "",
    [string]$TestDir = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent $ScriptDir

if ($Interpreter -eq "") {
    $Candidates = @(
        "$RepoRoot\build\release\cli\win\Release\nubasicdebug.exe",
        "$RepoRoot\build\release\cli\win\Debug\nubasicdebug.exe",
        "$RepoRoot\build\debug\cli\win\Debug\nubasicdebug.exe",
        "$RepoRoot\build\debug\cli\win\Release\nubasicdebug.exe"
    )
    foreach ($Candidate in $Candidates) {
        if (Test-Path $Candidate) {
            $Interpreter = $Candidate
            break
        }
    }
}

if ($Interpreter -eq "" -or -not (Test-Path $Interpreter)) {
    throw "nubasicdebug.exe not found. Build the project first, or pass -Interpreter <path>."
}

if ($TestDir -eq "") {
    $TestDir = $ScriptDir
}

$Interpreter = (Resolve-Path -Path $Interpreter).Path
$TestDir = (Resolve-Path -Path $TestDir).Path

Write-Host "Interruptible INPUT interpreter: $Interpreter"
Write-Host "Interruptible INPUT test dir    : $TestDir"

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $Interpreter
$psi.Arguments = "--machine-interface"
$psi.WorkingDirectory = $TestDir
$psi.UseShellExecute = $false
$psi.RedirectStandardInput = $true
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
$psi.CreateNoWindow = $false

$process = New-Object System.Diagnostics.Process
$process.StartInfo = $psi

try {
    if (-not $process.Start()) {
        throw "Failed to start $Interpreter"
    }

    Write-Host '>>> load "test_interruptible_input.bas"'
    $process.StandardInput.WriteLine('load "test_interruptible_input.bas"')
    $process.StandardInput.Flush()
    Start-Sleep -Milliseconds 150

    Write-Host '>>> run'
    $process.StandardInput.WriteLine('run')
    $process.StandardInput.Flush()
    Start-Sleep -Milliseconds 300

    Write-Host '>>> <ETX>'
    $process.StandardInput.Write([char]3)
    $process.StandardInput.Flush()
    Start-Sleep -Milliseconds 150

    Write-Host '>>> exit'
    $process.StandardInput.WriteLine('exit')
    $process.StandardInput.Flush()
    $process.StandardInput.Close()

    if (-not $process.WaitForExit(5000)) {
        throw "Interpreter did not exit after the interrupt contract scenario."
    }

    $output = $process.StandardOutput.ReadToEnd() + $process.StandardError.ReadToEnd()
    Write-Host $output

    if ($process.ExitCode -ne 0) {
        throw "Interpreter exited with code $($process.ExitCode)"
    }

    if ($output -notmatch '@@nubasic event="ready"') {
        throw "Debugger backend did not emit the ready event."
    }

    if ($output -notmatch '@@nubasic event="ok"') {
        throw "Debugger backend did not acknowledge the load command."
    }

    if ($output -notmatch 'Enter a value:') {
        throw "The INPUT prompt was not emitted before the interrupt."
    }

    if ($output -notmatch '@@nubasic event="stopped" reason="stop" line="20"') {
        throw "The INPUT instruction was not reported as a stoppable debug stop."
    }

    if ($output -match 'after input') {
        throw "Execution advanced past the INPUT instruction after the interrupt."
    }

    Write-Host ""
    Write-Host "PASS interruptible INPUT contract"
} finally {
    if (-not $process.HasExited) {
        try {
            $process.Kill()
        } catch {
        }
    }

    $process.Dispose()
}
