#Requires -Version 5.0
<#
.SYNOPSIS
    Removes installed nuBASIC MSI + VS Code extension, then reinstalls both.

.DESCRIPTION
    Intended for local developer use on Windows when iterating on the MSI and
    VS Code extension. The script:

    1. Finds and uninstalls installed nuBASIC MSI entries.
    2. Removes the `eantcal.nubasic` VS Code extension if VS Code is present.
    3. Installs the latest nuBASIC MSI from the build output (or an explicit one).
    4. Installs the requested VSIX (or the latest one in vscode-nubasic/).

    Run from an elevated PowerShell when uninstalling/installing the MSI.

.PARAMETER MsiPath
    Optional explicit path to the MSI to install.

.PARAMETER VsixPath
    Optional explicit path to the VSIX to install.

.PARAMETER SkipMsi
    Do not uninstall/reinstall the MSI.

.PARAMETER SkipVsix
    Do not uninstall/reinstall the VS Code extension.

.PARAMETER KeepInstalledMsi
    Skip MSI uninstall and only install the selected MSI.

.PARAMETER KeepInstalledVsix
    Skip VS Code extension uninstall and only install the selected VSIX.
#>
param(
    [string]$MsiPath = "",
    [string]$VsixPath = "",
    [switch]$SkipMsi,
    [switch]$SkipVsix,
    [switch]$KeepInstalledMsi,
    [switch]$KeepInstalledVsix
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent $ScriptDir
$ExtensionId = "eantcal.nubasic"

function Write-Section([string]$title) {
    Write-Host ""
    Write-Host ("=" * 72)
    Write-Host (" " + $title)
    Write-Host ("=" * 72)
}

function Resolve-LatestFile {
    param(
        [string]$Directory,
        [string]$Filter
    )

    if (-not (Test-Path $Directory)) {
        return $null
    }

    return Get-ChildItem -Path $Directory -Filter $Filter -File |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1
}

function Get-UninstallEntries {
    $roots = @(
        'HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall',
        'HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall',
        'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall'
    )

    foreach ($root in $roots) {
        if (-not (Test-Path $root)) {
            continue
        }

        foreach ($key in Get-ChildItem -Path $root -ErrorAction SilentlyContinue) {
            try {
                $props = Get-ItemProperty -Path $key.PSPath -ErrorAction Stop
                if ($props.DisplayName -like 'nuBASIC*') {
                    [PSCustomObject]@{
                        DisplayName = $props.DisplayName
                        DisplayVersion = $props.DisplayVersion
                        ProductCode = $key.PSChildName
                        QuietUninstallString = $props.QuietUninstallString
                        UninstallString = $props.UninstallString
                    }
                }
            }
            catch {
            }
        }
    }
}

function Get-VsCodeCli {
    $localAppData = [Environment]::GetFolderPath('LocalApplicationData')
    $programFiles = [Environment]::GetFolderPath('ProgramFiles')
    $programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')

    $candidates = @(
        "$localAppData\Programs\Microsoft VS Code\bin\code.cmd",
        "$programFiles\Microsoft VS Code\bin\code.cmd",
        "$programFilesX86\Microsoft VS Code\bin\code.cmd"
    )

    $fromPath = Get-Command code.cmd -ErrorAction SilentlyContinue
    if ($fromPath) {
        $candidates += $fromPath.Source
    }

    return $candidates |
        Where-Object { $_ -and (Test-Path $_) } |
        Select-Object -First 1
}

function Invoke-Msiexec {
    param([string[]]$Arguments)

    $argString = $Arguments -join ' '
    Write-Host "Running: msiexec.exe $argString"
    $proc = Start-Process -FilePath "msiexec.exe" -ArgumentList $Arguments -Wait -PassThru
    if ($proc.ExitCode -ne 0) {
        throw "msiexec failed with exit code $($proc.ExitCode)."
    }
}

if (-not $SkipMsi) {
    Write-Section "nuBASIC MSI"

    if (-not $KeepInstalledMsi) {
        $entries = @(Get-UninstallEntries)
        if ($entries.Count -eq 0) {
            Write-Host "No installed nuBASIC MSI entries found."
        } else {
            foreach ($entry in $entries) {
                Write-Host "Uninstalling: $($entry.DisplayName) $($entry.DisplayVersion)"
                if ($entry.ProductCode -match '^\{.+\}$') {
                    Invoke-Msiexec -Arguments @('/x', $entry.ProductCode, '/passive', '/norestart')
                } elseif ($entry.QuietUninstallString) {
                    Write-Host "Running quiet uninstall string."
                    & cmd.exe /d /s /c $entry.QuietUninstallString
                    if ($LASTEXITCODE -ne 0) {
                        throw "Quiet uninstall failed with exit code $LASTEXITCODE."
                    }
                } elseif ($entry.UninstallString) {
                    Write-Host "Running uninstall string."
                    & cmd.exe /d /s /c $entry.UninstallString
                    if ($LASTEXITCODE -ne 0) {
                        throw "Uninstall failed with exit code $LASTEXITCODE."
                    }
                } else {
                    throw "No uninstall command found for $($entry.DisplayName)."
                }
            }
        }
    } else {
        Write-Host "Keeping currently installed MSI."
    }

    if (-not $MsiPath) {
        $latestMsi = Resolve-LatestFile -Directory "$RepoRoot\build\release" -Filter "nuBASIC_*.msi"
        if ($latestMsi) {
            $MsiPath = $latestMsi.FullName
        }
    }

    if (-not $MsiPath) {
        throw "No MSI specified and none found under $RepoRoot\build\release."
    }

    $MsiPath = (Resolve-Path -Path $MsiPath).Path
    Write-Host "Installing MSI: $MsiPath"
    Invoke-Msiexec -Arguments @('/i', "`"$MsiPath`"", '/passive', '/norestart')
}

if (-not $SkipVsix) {
    Write-Section "VS Code Extension"

    $code = Get-VsCodeCli
    if (-not $code) {
        throw "VS Code CLI (code.cmd) not found."
    }

    Write-Host "Using VS Code CLI: $code"

    if (-not $KeepInstalledVsix) {
        Write-Host "Removing extension: $ExtensionId"
        & $code --uninstall-extension $ExtensionId --force
        if ($LASTEXITCODE -ne 0) {
            Write-Host "VS Code uninstall returned exit code $LASTEXITCODE; continuing."
        }
    } else {
        Write-Host "Keeping currently installed VS Code extension."
    }

    if (-not $VsixPath) {
        $latestVsix = Resolve-LatestFile -Directory "$RepoRoot\vscode-nubasic" -Filter "nubasic-*.vsix"
        if ($latestVsix) {
            $VsixPath = $latestVsix.FullName
        }
    }

    if (-not $VsixPath) {
        throw "No VSIX specified and none found under $RepoRoot\vscode-nubasic."
    }

    $VsixPath = (Resolve-Path -Path $VsixPath).Path
    Write-Host "Installing VSIX: $VsixPath"
    & $code --install-extension $VsixPath --force
    if ($LASTEXITCODE -ne 0) {
        throw "VS Code extension install failed with exit code $LASTEXITCODE."
    }
}

Write-Section "Done"
Write-Host "nuBASIC reinstall flow completed."
