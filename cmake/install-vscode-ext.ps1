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

    $vsix = Join-Path $InstallDir "bin\nubasic-0.1.0.vsix"
    Write-Host "VSIX path  : $vsix"

    if (-not (Test-Path $vsix)) {
        Write-Host "VSIX not found — skipping."
        exit 0
    }

    # Use .NET folder resolution — $env:LOCALAPPDATA may be unset in deferred CAs.
    $localAppData = [System.Environment]::GetFolderPath('LocalApplicationData')
    $programFiles = [System.Environment]::GetFolderPath('ProgramFiles')

    $candidates = [System.Collections.Generic.List[string]]@(
        "$localAppData\Programs\Microsoft VS Code\bin\code.cmd",
        "$programFiles\Microsoft VS Code\bin\code.cmd"
    )
    $fromPath = Get-Command code.cmd -ErrorAction SilentlyContinue
    if ($fromPath) { $candidates.Add($fromPath.Source) }

    Write-Host "Searching for code.cmd..."
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
