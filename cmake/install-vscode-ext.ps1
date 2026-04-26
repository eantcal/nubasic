#Requires -Version 5.0
<#
.SYNOPSIS
    Installs the nuBASIC VS Code extension into the current user's VS Code.
    Called as a deferred MSI custom action after InstallFinalize.

.PARAMETER InstallDir
    nuBASIC installation root (the directory that contains bin\).
#>
param([string]$InstallDir = $PSScriptRoot)

$InstallDir = $InstallDir.TrimEnd('"', '\', ' ')

$log = Join-Path $env:TEMP 'nubasic-vscode-install.log'
Start-Transcript -Path $log -Force | Out-Null

function Show-InstallerError([string]$message) {
    try {
        Add-Type -AssemblyName System.Windows.Forms
        [System.Windows.Forms.MessageBox]::Show(
            $message,
            'nuBASIC VS Code Extension',
            [System.Windows.Forms.MessageBoxButtons]::OK,
            [System.Windows.Forms.MessageBoxIcon]::Error
        ) | Out-Null
    }
    catch {
        Write-Host "Could not display MessageBox: $_"
    }
}

try {
    Write-Host "nuBASIC VS Code extension installer"
    Write-Host "InstallDir : $InstallDir"
    Write-Host "User       : $env:USERNAME"
    Write-Host "Time       : $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"

    $binDir = Join-Path $InstallDir "bin"
    $vsix = Get-ChildItem -Path $binDir -Filter "nubasic-*.vsix" -File -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1 -ExpandProperty FullName
    Write-Host "VSIX path  : $vsix"

    if (-not $vsix -or -not (Test-Path $vsix)) {
        Write-Host "VSIX not found in $binDir - skipping."
        exit 0
    }

    $localAppData    = [System.Environment]::GetFolderPath('LocalApplicationData')
    $programFiles    = [System.Environment]::GetFolderPath('ProgramFiles')
    $programFilesX86 = [System.Environment]::GetFolderPath('ProgramFilesX86')

    $candidates = [System.Collections.Generic.List[string]]::new()
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

    foreach ($root in @(
        "$localAppData\Programs\Microsoft VS Code",
        "$programFiles\Microsoft VS Code",
        "$programFilesX86\Microsoft VS Code"
    )) {
        $candidates.Add("$root\bin\code.cmd")
        $candidates.Add("$root\Code.exe")
    }

    $fromPath = Get-Command code.cmd -ErrorAction SilentlyContinue
    if ($fromPath) { $candidates.Add($fromPath.Source) }

    Write-Host "Searching for VS Code..."
    foreach ($c in $candidates) { Write-Host "  candidate: $c" }

    $code = $candidates |
        Where-Object { $_ -and (Test-Path $_ -ErrorAction SilentlyContinue) } |
        Select-Object -First 1

    if (-not $code) {
        Write-Host "VS Code not found - extension not installed."
        exit 0
    }

    Write-Host "Found: $code"
    Write-Host "Running: & `"$code`" --install-extension `"$vsix`" --force"
    $installOutput = & $code --install-extension $vsix --force 2>&1 | Out-String
    if ($installOutput) {
        Write-Host "--- VS Code CLI output ---"
        Write-Host $installOutput.TrimEnd()
        Write-Host "--- end VS Code CLI output ---"
    }
    Write-Host "Exit code: $LASTEXITCODE"
    if ($LASTEXITCODE -ne 0) {
        throw "VS Code CLI failed with exit code $LASTEXITCODE."
    }
}
catch {
    $details = $_ | Out-String
    Write-Host "ERROR: $details"
    Show-InstallerError(
        "VS Code extension installation failed.`r`n`r`n" +
        $details.Trim() +
        "`r`n`r`nFull log: $log"
    )
}
finally {
    Stop-Transcript | Out-Null
}
