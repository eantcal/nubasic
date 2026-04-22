#Requires -Version 5.0
<#
.SYNOPSIS
    Installs the nuBASIC VS Code extension into the current user's VS Code.
    Called as a deferred MSI custom action after InstallFinalize.

.PARAMETER InstallDir
    nuBASIC installation root (the directory that contains bin\).
#>
param([string]$InstallDir = $PSScriptRoot)

# [INSTALL_ROOT] ends with \, so the WiX ExeCommand produces "path\" where the
# backslash escapes the closing quote, leaving a trailing " in the parsed value.
$InstallDir = $InstallDir.TrimEnd('"', '\', ' ')

$log = Join-Path $env:TEMP 'nubasic-vscode-install.log'
Start-Transcript -Path $log -Force | Out-Null

try {
    Write-Host "nuBASIC VS Code extension installer"
    Write-Host "InstallDir : $InstallDir"
    Write-Host "User       : $env:USERNAME"
    Write-Host "Time       : $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"

    $vsix = Join-Path $InstallDir "bin\nubasic-0.1.2.vsix"
    Write-Host "VSIX path  : $vsix"

    if (-not (Test-Path $vsix)) {
        Write-Host "VSIX not found — skipping."
        exit 0
    }

    # Use .NET APIs — env vars may be unset in deferred MSI custom actions.
    $localAppData    = [System.Environment]::GetFolderPath('LocalApplicationData')
    $programFiles    = [System.Environment]::GetFolderPath('ProgramFiles')
    $programFilesX86 = [System.Environment]::GetFolderPath('ProgramFilesX86')

    $candidates = [System.Collections.Generic.List[string]]::new()

    # --- Registry lookup (most reliable) ---
    $regBases = @(
        'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall',
        'HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall',
        'HKLM:\Software\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall'
    )
    foreach ($base in $regBases) {
        if (-not (Test-Path $base)) { continue }
        $key = Get-ChildItem $base -ErrorAction SilentlyContinue |
            Where-Object { $_.GetValue('DisplayName') -like '*Visual Studio Code*' } |
            Select-Object -First 1
        if ($key) {
            $loc = $key.GetValue('InstallLocation')
            if ($loc) {
                $loc = $loc.TrimEnd('\')
                $candidates.Add("$loc\bin\code.cmd")
                $candidates.Add("$loc\Code.exe")
                Write-Host "Registry install location: $loc"
            }
        }
    }

    # --- Common default paths ---
    foreach ($root in @(
        "$localAppData\Programs\Microsoft VS Code",
        "$programFiles\Microsoft VS Code",
        "$programFilesX86\Microsoft VS Code"
    )) {
        $candidates.Add("$root\bin\code.cmd")
        $candidates.Add("$root\Code.exe")
    }

    # --- PATH fallback ---
    $fromPath = Get-Command code.cmd -ErrorAction SilentlyContinue
    if ($fromPath) { $candidates.Add($fromPath.Source) }

    Write-Host "Searching for VS Code..."
    foreach ($c in $candidates) { Write-Host "  candidate: $c" }

    $code = $candidates |
        Where-Object { $_ -and (Test-Path $_ -ErrorAction SilentlyContinue) } |
        Select-Object -First 1

    if (-not $code) {
        Write-Host "VS Code not found — extension not installed."
        exit 0
    }

    Write-Host "Found: $code"
    Write-Host "Running: & `"$code`" --install-extension `"$vsix`" --force"
    & $code --install-extension $vsix --force
    Write-Host "Exit code: $LASTEXITCODE"
}
catch {
    Write-Host "ERROR: $_"
}
finally {
    Stop-Transcript | Out-Null
}
