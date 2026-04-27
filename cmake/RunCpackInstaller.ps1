#Requires -Version 5.0
param(
    [Parameter(Mandatory)] [string] $CPackExe,
    [Parameter(Mandatory)] [string] $Generator,
    [Parameter(Mandatory)] [string] $Config,
    [Parameter(Mandatory)] [string] $ConfigFile,
    [Parameter(Mandatory)] [string] $BinaryDir,
    [Parameter(Mandatory)] [string] $SourceDir,
    [Parameter(Mandatory)] [string] $MsiOutput,
    [string] $Candle,
    [string] $Light
)

$ErrorActionPreference = 'Stop'

$configText = Get-Content -LiteralPath $ConfigFile -Raw -Encoding UTF8
$timestamp = [DateTime]::UtcNow.ToString('yyyyMMddHHmmssfff')
$topLevelTag = "win64-$timestamp-$PID"
$patchedConfig = [regex]::Replace(
    $configText,
    'set\(CPACK_TOPLEVEL_TAG\s+"[^"]*"\)',
    "set(CPACK_TOPLEVEL_TAG `"$topLevelTag`")",
    1
)

$tempConfig = Join-Path $BinaryDir "CPackConfig.$topLevelTag.cmake"
Set-Content -LiteralPath $tempConfig -Value $patchedConfig -Encoding UTF8

if (Test-Path -LiteralPath $MsiOutput) {
    Remove-Item -LiteralPath $MsiOutput -Force
}

try {
    & $CPackExe -G $Generator -C $Config --config $tempConfig
    if ($LASTEXITCODE -ne 0) {
        throw "cpack failed with exit code $LASTEXITCODE"
    }

    if ($Generator -ieq 'WIX' -and $Candle -and $Light) {
        $wixPkgDir = Join-Path $BinaryDir "_CPack_Packages\$topLevelTag\WIX"
        & powershell -NoProfile -ExecutionPolicy Bypass `
            -File (Join-Path $SourceDir 'cmake\PatchWixMSI.ps1') `
            -WixPkgDir $wixPkgDir `
            -Candle $Candle `
            -Light $Light `
            -MsiOutput $MsiOutput

        if ($LASTEXITCODE -ne 0) {
            throw "PatchWixMSI.ps1 failed with exit code $LASTEXITCODE"
        }
    }
} finally {
    if (Test-Path -LiteralPath $tempConfig) {
        Remove-Item -LiteralPath $tempConfig -Force -ErrorAction SilentlyContinue
    }
}
