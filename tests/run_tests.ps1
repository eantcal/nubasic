# run_tests.ps1 — nuBASIC regression test runner (PowerShell)
#
# Usage:
#   .\run_tests.ps1 [-Interpreter <path>] [-TestDir <path>]
#
# If -Interpreter is not given, the script searches common build output
# locations relative to the repo root.
#
# Exit code: 0 = all tests passed, 1 = one or more failures.

param(
    [string]$Interpreter = "",
    [string]$TestDir     = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$LASTEXITCODE = 0   # initialise so StrictMode does not complain before the first external call

# ── locate repo root ─────────────────────────────────────────────────────────
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot  = Split-Path -Parent $ScriptDir

# ── locate interpreter ───────────────────────────────────────────────────────
if ($Interpreter -eq "") {
    $Candidates = @(
        "$RepoRoot\build\release\Release\nubasic.exe",
        "$RepoRoot\build\debug\Debug\nubasic.exe",
        "$RepoRoot\x64\Release\nubasic.exe",
        "$RepoRoot\x64\Debug\nubasic.exe",
        "$RepoRoot\out\Release\nubasic.exe",
        "$RepoRoot\out\Debug\nubasic.exe"
    )
    foreach ($c in $Candidates) {
        if (Test-Path $c) { $Interpreter = $c; break }
    }
}

if ($Interpreter -eq "" -or -not (Test-Path $Interpreter)) {
    Write-Error "nubasic.exe not found. Build the project first, or pass -Interpreter <path>."
    exit 1
}

# ── locate test directory ────────────────────────────────────────────────────
if ($TestDir -eq "") { $TestDir = $ScriptDir }
$TestFiles = Get-ChildItem -Path $TestDir -Filter "test_*.bas" | Sort-Object Name

if ($TestFiles.Count -eq 0) {
    Write-Error "No test_*.bas files found in $TestDir"
    exit 1
}

# ── run tests ────────────────────────────────────────────────────────────────
$TotalPass   = 0
$TotalFail   = 0
$SuitePass   = 0
$SuiteFail   = 0
$FailedSuites = @()

$Width = 72
$Sep   = "-" * $Width

Write-Host ""
Write-Host ("=" * $Width)
Write-Host " nuBASIC Regression Test Suite"
Write-Host " Interpreter : $Interpreter"
Write-Host " Test dir    : $TestDir"
Write-Host ("=" * $Width)

foreach ($f in $TestFiles) {
    $Name   = $f.BaseName
    $Path   = $f.FullName

    Write-Host ""
    Write-Host $Sep
    Write-Host " Suite: $Name"
    Write-Host $Sep

    # Run interpreter in text mode (-t) so I/O goes to stdout
    $Output = & "$Interpreter" -t -e "$Path" 2>&1
    $ExitCode = $LASTEXITCODE

    # Echo all output (PASS/FAIL lines visible in CI logs)
    foreach ($line in $Output) { Write-Host "  $line" }

    # Parse pass/fail counts from the summary line:
    #   "Results: N passed,  M failed"
    $SuiteP = 0
    $SuiteF = 0
    foreach ($line in $Output) {
        if ($line -match "Results:\s*(\d+)\s*passed[^0-9]+(\d+)\s*failed") {
            $SuiteP = [int]$Matches[1]
            $SuiteF = [int]$Matches[2]
            break
        }
    }

    # Fallback: count PASS/FAIL lines directly
    if ($SuiteP -eq 0 -and $SuiteF -eq 0) {
        foreach ($line in $Output) {
            if ($line -match "^\s*PASS\b")  { $SuiteP++ }
            if ($line -match "^\s*FAIL\b")  { $SuiteF++ }
        }
    }

    $TotalPass += $SuiteP
    $TotalFail += $SuiteF

    if ($SuiteF -eq 0 -and $ExitCode -eq 0) {
        Write-Host "  [SUITE PASS] $Name  ($SuiteP assertions passed)"
        $SuitePass++
    } else {
        Write-Host "  [SUITE FAIL] $Name  ($SuiteP passed, $SuiteF failed, exit=$ExitCode)"
        $SuiteFail++
        $FailedSuites += $Name
    }
}

# ── final summary ─────────────────────────────────────────────────────────────
Write-Host ""
Write-Host ("=" * $Width)
Write-Host " SUMMARY"
Write-Host ("=" * $Width)
Write-Host "  Suites  : $($SuitePass + $SuiteFail) total,  $SuitePass passed,  $SuiteFail failed"
Write-Host "  Asserts : $($TotalPass + $TotalFail) total,  $TotalPass passed,  $TotalFail failed"
if ($FailedSuites.Count -gt 0) {
    Write-Host "  Failed suites:"
    foreach ($s in $FailedSuites) { Write-Host "    - $s" }
}
Write-Host ("=" * $Width)

if ($TotalFail -eq 0 -and $SuiteFail -eq 0) {
    Write-Host " ALL TESTS PASSED"
    Write-Host ("=" * $Width)
    exit 0
} else {
    Write-Host " SOME TESTS FAILED"
    Write-Host ("=" * $Width)
    exit 1
}
