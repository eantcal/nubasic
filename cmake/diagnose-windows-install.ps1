#Requires -Version 5.0
<#
.SYNOPSIS
    Collects diagnostics for a nuBASIC Windows installation.

.DESCRIPTION
    Read-only support script for cases where the MSI installs but the installed
    binaries do not start, often because a runtime DLL is missing on a clean
    Windows 10/11 machine.

.PARAMETER InstallDir
    nuBASIC install root. If omitted, registry keys and Program Files are used.

.PARAMETER MsiLog
    Optional verbose msiexec log produced with /L*V.
#>
param(
    [string] $InstallDir,
    [string] $MsiLog,
    [string] $OutputDir
)

$ErrorActionPreference = 'Continue'

function Test-IsAdmin {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Get-NuBasicUninstallEntries {
    foreach ($base in @(
        'HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall',
        'HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall',
        'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall')) {
        if (-not (Test-Path $base)) { continue }
        Get-ChildItem $base -ErrorAction SilentlyContinue | ForEach-Object {
            $p = Get-ItemProperty -LiteralPath $_.PSPath -ErrorAction SilentlyContinue
            if ($p.DisplayName -like '*nuBASIC*') {
                [pscustomobject]@{
                    Hive = $base
                    ProductCode = $_.PSChildName
                    DisplayName = $p.DisplayName
                    DisplayVersion = $p.DisplayVersion
                    InstallLocation = $p.InstallLocation
                    UninstallString = $p.UninstallString
                }
            }
        }
    }
}

function Resolve-InstallDir {
    if ($InstallDir -and (Test-Path -LiteralPath $InstallDir)) {
        return (Resolve-Path -LiteralPath $InstallDir).Path
    }
    foreach ($key in 'HKLM:\Software\nuBASIC', 'HKCU:\Software\nuBASIC') {
        if (Test-Path $key) {
            $p = Get-ItemProperty -LiteralPath $key -ErrorAction SilentlyContinue
            if ($p.InstallDir -and (Test-Path -LiteralPath $p.InstallDir)) {
                return (Resolve-Path -LiteralPath $p.InstallDir).Path
            }
        }
    }
    foreach ($entry in Get-NuBasicUninstallEntries) {
        if ($entry.InstallLocation -and (Test-Path -LiteralPath $entry.InstallLocation)) {
            return (Resolve-Path -LiteralPath $entry.InstallLocation).Path
        }
    }
    $default = Join-Path ([Environment]::GetFolderPath('ProgramFiles')) 'nuBASIC'
    if (Test-Path -LiteralPath $default) {
        return (Resolve-Path -LiteralPath $default).Path
    }
    $null
}

function Write-CapturedFile([string] $Name, [scriptblock] $Body) {
    $path = Join-Path $OutputDir $Name
    try {
        & $Body 2>&1 | Out-String | Set-Content -LiteralPath $path -Encoding UTF8
    }
    catch {
        "ERROR: $_" | Set-Content -LiteralPath $path -Encoding UTF8
    }
}

function Invoke-Smoke([string] $Exe, [string[]] $Arguments) {
    Write-Host "> $Exe $($Arguments -join ' ')"
    if (-not (Test-Path -LiteralPath $Exe)) {
        Write-Host "MISSING: $Exe"
        return
    }
    try {
        $out = & $Exe @Arguments 2>&1 | Out-String
        if ($out) { Write-Host $out.TrimEnd() }
        Write-Host "ExitCode=$LASTEXITCODE"
    }
    catch {
        Write-Host "ERROR: $_"
    }
}

if (-not $OutputDir) {
    $OutputDir = Join-Path $env:TEMP ("nubasic-install-diagnostics-" + (Get-Date -Format 'yyyyMMdd-HHmmss'))
}
try {
    New-Item -ItemType Directory -Path $OutputDir -Force -ErrorAction Stop | Out-Null
    $OutputDir = (Resolve-Path -LiteralPath $OutputDir -ErrorAction Stop).Path
}
catch {
    Write-Host "Could not create requested OutputDir '$OutputDir': $_"
    $OutputDir = Join-Path $env:TEMP ("nubasic-install-diagnostics-" + (Get-Date -Format 'yyyyMMdd-HHmmss'))
    New-Item -ItemType Directory -Path $OutputDir -Force -ErrorAction Stop | Out-Null
    $OutputDir = (Resolve-Path -LiteralPath $OutputDir -ErrorAction Stop).Path
}
$InstallDir = Resolve-InstallDir
$BinDir = if ($InstallDir) { Join-Path $InstallDir 'bin' } else { $null }

$transcriptStarted = $false
try {
    Start-Transcript -Path (Join-Path $OutputDir 'transcript.txt') -Force -ErrorAction Stop | Out-Null
    $transcriptStarted = $true
}
catch {
    Write-Host "Could not start transcript: $_"
}
try {
    Write-Host "nuBASIC install diagnostics"
    Write-Host "OutputDir : $OutputDir"
    Write-Host "InstallDir: $InstallDir"

    Write-CapturedFile 'system.txt' {
        Get-CimInstance Win32_OperatingSystem |
            Select-Object Caption, Version, BuildNumber, OSArchitecture |
            Format-List
        [pscustomobject]@{
            User = "$env:USERDOMAIN\$env:USERNAME"
            IsAdmin = Test-IsAdmin
            PowerShell = $PSVersionTable.PSVersion.ToString()
            Process64Bit = [Environment]::Is64BitProcess
            OS64Bit = [Environment]::Is64BitOperatingSystem
            PATH = $env:PATH
        } | Format-List
    }

    Write-CapturedFile 'registry.txt' {
        'Uninstall entries:'
        Get-NuBasicUninstallEntries | Format-List
        'nuBASIC keys:'
        foreach ($key in 'HKLM:\Software\nuBASIC', 'HKCU:\Software\nuBASIC') {
            $key
            if (Test-Path $key) { Get-ItemProperty -LiteralPath $key | Format-List } else { 'Missing' }
        }
    }

    Write-CapturedFile 'installed-bin.txt' {
        if (-not $BinDir -or -not (Test-Path -LiteralPath $BinDir)) {
            "Missing bin directory: $BinDir"
            return
        }
        Get-ChildItem -LiteralPath $BinDir -File |
            Select-Object Name, Length, LastWriteTime |
            Sort-Object Name |
            Format-Table -AutoSize

        ''
        'Expected runtime DLLs:'
        $expectedDlls = foreach ($dll in 'ffi-8.dll','ffi.dll','vcruntime140.dll','vcruntime140_1.dll',
                       'msvcp140.dll','msvcp140_1.dll','msvcp140_2.dll','concrt140.dll',
                       'SciLexer.dll') {
            [pscustomobject]@{
                Name = $dll
                Present = Test-Path -LiteralPath (Join-Path $BinDir $dll)
            }
        }
        $expectedDlls | Format-Table -AutoSize
    }

    Write-CapturedFile 'smoke-tests.txt' {
        if (-not $BinDir) {
            'Install directory not resolved.'
            return
        }
        $smoke = Join-Path $OutputDir 'smoke.bas'
        'Print "nuBASIC diagnostic smoke"' | Set-Content -LiteralPath $smoke -Encoding ASCII
        'Print Ver$()' | Add-Content -LiteralPath $smoke -Encoding ASCII
        Invoke-Smoke (Join-Path $BinDir 'nubasic.exe') @('--version')
        Invoke-Smoke (Join-Path $BinDir 'nubasic.exe') @('-t', '-e', $smoke)
        Invoke-Smoke (Join-Path $BinDir 'nubasicdebug.exe') @('--version')
    }

    Write-CapturedFile 'msi-events.txt' {
        Get-WinEvent -FilterHashtable @{
            LogName = 'Application'
            ProviderName = 'MsiInstaller'
            StartTime = (Get-Date).AddDays(-14)
        } -ErrorAction SilentlyContinue |
            Select-Object TimeCreated, Id, LevelDisplayName, Message |
            Format-List
    }

    if ($MsiLog -and (Test-Path -LiteralPath $MsiLog)) {
        Copy-Item -LiteralPath $MsiLog -Destination (Join-Path $OutputDir 'msiexec-verbose.log') -Force
    }
    $vscodeLog = Join-Path $env:TEMP 'nubasic-vscode-install.log'
    if (Test-Path -LiteralPath $vscodeLog) {
        Copy-Item -LiteralPath $vscodeLog -Destination (Join-Path $OutputDir 'nubasic-vscode-install.log') -Force
    }
}
finally {
    if ($transcriptStarted) { Stop-Transcript | Out-Null }
}

$zip = "$OutputDir.zip"
if (Test-Path -LiteralPath $zip) {
    Remove-Item -LiteralPath $zip -Force
}
Compress-Archive -Path (Join-Path $OutputDir '*') -DestinationPath $zip -Force
Write-Host "Diagnostic bundle: $zip"
