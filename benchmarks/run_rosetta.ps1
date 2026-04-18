param(
    [string]$Interpreter = "",
    [string]$Script = "",
    [int]$Iterations = 5,
    [int]$Warmup = 1
)

$ErrorActionPreference = "Stop"

function Quote-Arg([string]$Value) {
    if ($Value.Length -eq 0) {
        return '""'
    }

    if ($Value -notmatch '[\s"]') {
        return $Value
    }

    $escaped = $Value -replace '(\\*)"', '$1$1\"'
    $escaped = $escaped -replace '(\\+)$', '$1$1'
    return '"' + $escaped + '"'
}

function Resolve-RepoRoot {
    $scriptDir = Split-Path -Parent $PSCommandPath
    return (Resolve-Path (Join-Path $scriptDir "..")).Path
}

function Resolve-Interpreter([string]$RepoRoot, [string]$Requested) {
    if ($Requested) {
        $resolved = Resolve-Path $Requested -ErrorAction SilentlyContinue
        if ($resolved) {
            return $resolved.Path
        }
        throw "Interpreter not found: $Requested"
    }

    $candidates = @(
        "$RepoRoot\build\release\cli\win\Release\nubasic.exe",
        "$RepoRoot\build\debug\cli\win\Debug\nubasic.exe",
        "$RepoRoot\build\release\Release\nubasic.exe",
        "$RepoRoot\build\debug\Debug\nubasic.exe",
        "$RepoRoot\x64\Release\nubasic.exe",
        "$RepoRoot\x64\Debug\nubasic.exe"
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return (Resolve-Path $candidate).Path
        }
    }

    throw "nubasic.exe not found. Build nuBasicCLI first, or pass -Interpreter <path>."
}

function Invoke-BenchmarkRun(
    [string]$InterpreterPath,
    [string[]]$Arguments,
    [string]$WorkingDirectory
) {
    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.FileName = $InterpreterPath
    $psi.Arguments = ($Arguments | ForEach-Object { Quote-Arg $_ }) -join " "
    $psi.WorkingDirectory = $WorkingDirectory
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true

    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $proc = [System.Diagnostics.Process]::Start($psi)
    $stdout = $proc.StandardOutput.ReadToEnd()
    $stderr = $proc.StandardError.ReadToEnd()
    $proc.WaitForExit()
    $sw.Stop()

    [pscustomobject]@{
        ExitCode = $proc.ExitCode
        Milliseconds = [math]::Round($sw.Elapsed.TotalMilliseconds, 3)
        Stdout = $stdout
        Stderr = $stderr
    }
}

$repoRoot = Resolve-RepoRoot

if ($Iterations -lt 1) {
    throw "Iterations must be at least 1."
}

if ($Warmup -lt 0) {
    throw "Warmup must be 0 or greater."
}

if (-not $Script) {
    $Script = Join-Path $repoRoot "examples\rosetta.bas"
}

$scriptPath = Resolve-Path $Script -ErrorAction Stop
$interpreterPath = Resolve-Interpreter $repoRoot $Interpreter
$workingDirectory = Split-Path -Parent $scriptPath.Path

$argsList = @("-t")
$argsList += "-e"
$argsList += $scriptPath.Path

Write-Host "nuBASIC Rosetta benchmark"
Write-Host "Interpreter : $interpreterPath"
Write-Host "Script      : $($scriptPath.Path)"
Write-Host "Mode        : text/headless (-t)"
Write-Host "Warmup      : $Warmup"
Write-Host "Iterations  : $Iterations"
Write-Host ""

for ($i = 1; $i -le $Warmup; ++$i) {
    $run = Invoke-BenchmarkRun $interpreterPath $argsList $workingDirectory
    if ($run.ExitCode -ne 0) {
        Write-Error "Warmup $i failed with exit code $($run.ExitCode)`n$($run.Stdout)`n$($run.Stderr)"
    }
    Write-Host ("warmup {0,2}: {1,10:N3} ms" -f $i, $run.Milliseconds)
}

$samples = New-Object System.Collections.Generic.List[double]

for ($i = 1; $i -le $Iterations; ++$i) {
    $run = Invoke-BenchmarkRun $interpreterPath $argsList $workingDirectory
    if ($run.ExitCode -ne 0) {
        Write-Error "Iteration $i failed with exit code $($run.ExitCode)`n$($run.Stdout)`n$($run.Stderr)"
    }
    $samples.Add([double]$run.Milliseconds)
    Write-Host ("run    {0,2}: {1,10:N3} ms" -f $i, $run.Milliseconds)
}

$ordered = $samples | Sort-Object
$count = $ordered.Count
$min = $ordered[0]
$max = $ordered[$count - 1]
$avg = ($samples | Measure-Object -Average).Average

if (($count % 2) -eq 1) {
    $median = $ordered[[int][math]::Floor($count / 2)]
} else {
    $upper = [int]($count / 2)
    $median = ($ordered[$upper - 1] + $ordered[$upper]) / 2.0
}

Write-Host ""
Write-Host ("min    : {0,10:N3} ms" -f $min)
Write-Host ("median : {0,10:N3} ms" -f $median)
Write-Host ("avg    : {0,10:N3} ms" -f $avg)
Write-Host ("max    : {0,10:N3} ms" -f $max)
