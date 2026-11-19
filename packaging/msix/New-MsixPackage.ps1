[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $StageDir,

    [Parameter(Mandatory = $true)]
    [string] $OutputDir,

    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[1-9][0-9]{0,4}\.[0-9]{1,5}\.[0-9]{1,5}\.0$')]
    [string] $Version,

    [string] $WorkingDir = '',

    [switch] $SignForTesting
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$packageIdentityName = 'AntoninoCalderone.nuBASIC'
$packagePublisher = 'CN=006FC507-ECB1-41CF-9858-405FEB4A81E5'

function Get-WindowsSdkTool {
    param([Parameter(Mandatory = $true)][string] $Name)

    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
    $sdkBin = Join-Path $programFilesX86 'Windows Kits\10\bin'
    if (-not (Test-Path -LiteralPath $sdkBin)) {
        throw "Windows SDK bin directory was not found: $sdkBin"
    }

    $candidate = Get-ChildItem -LiteralPath $sdkBin -Directory |
        Sort-Object Name -Descending |
        ForEach-Object { Join-Path $_.FullName "x64\$Name" } |
        Where-Object { Test-Path -LiteralPath $_ } |
        Select-Object -First 1

    if (-not $candidate) {
        throw "$Name was not found in the Windows SDK."
    }

    return $candidate
}

function Assert-SafeWorkPath {
    param([Parameter(Mandatory = $true)][string] $Path)

    $fullPath = [IO.Path]::GetFullPath($Path)
    $root = [IO.Path]::GetPathRoot($fullPath)
    if ($fullPath -eq $root -or $fullPath.Length -lt ($root.Length + 8)) {
        throw "Refusing to use an unsafe MSIX working path: $fullPath"
    }
    return $fullPath
}

function Assert-MsixSignature {
    param(
        [Parameter(Mandatory = $true)][string] $PackagePath,
        [Parameter(Mandatory = $true)][string] $ExpectedThumbprint
    )

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    Add-Type -AssemblyName System.Security.Cryptography.Pkcs

    $archive = [IO.Compression.ZipFile]::OpenRead($PackagePath)
    try {
        $signatureEntry = $archive.GetEntry('AppxSignature.p7x')
        if (-not $signatureEntry) {
            throw 'The signed MSIX does not contain AppxSignature.p7x.'
        }

        $memory = [IO.MemoryStream]::new()
        try {
            $stream = $signatureEntry.Open()
            try {
                $stream.CopyTo($memory)
            }
            finally {
                $stream.Dispose()
            }
            [byte[]] $signatureBytes = $memory.ToArray()
        }
        finally {
            $memory.Dispose()
        }
    }
    finally {
        $archive.Dispose()
    }

    if ($signatureBytes.Length -le 4 -or
        [Text.Encoding]::ASCII.GetString($signatureBytes, 0, 4) -ne 'PKCX') {
        throw 'AppxSignature.p7x does not have the expected MSIX signature header.'
    }

    [byte[]] $cmsBytes = $signatureBytes[4..($signatureBytes.Length - 1)]
    $signedCms = [Security.Cryptography.Pkcs.SignedCms]::new()
    $signedCms.Decode($cmsBytes)
    $signedCms.CheckSignature($true)

    if ($signedCms.SignerInfos.Count -ne 1) {
        throw "Expected one MSIX signer, found $($signedCms.SignerInfos.Count)."
    }

    $actualThumbprint = $signedCms.SignerInfos[0].Certificate.Thumbprint
    if ($actualThumbprint -ne $ExpectedThumbprint) {
        throw "MSIX signer '$actualThumbprint' does not match '$ExpectedThumbprint'."
    }
}

$stagePath = [IO.Path]::GetFullPath($StageDir)
if (-not (Test-Path -LiteralPath $stagePath -PathType Container)) {
    throw "The staged install directory does not exist: $stagePath"
}

$requiredFiles = @(
    'bin\NuBasicIDE.exe',
    'bin\nubasic.exe'
)
foreach ($relativePath in $requiredFiles) {
    $requiredPath = Join-Path $stagePath $relativePath
    if (-not (Test-Path -LiteralPath $requiredPath -PathType Leaf)) {
        throw "Required MSIX payload file is missing: $requiredPath"
    }
}

$versionParts = $Version.Split('.')
foreach ($part in $versionParts) {
    if ([int]$part -gt 65535) {
        throw "Each MSIX version component must be at most 65535: $Version"
    }
}

$outputPath = [IO.Path]::GetFullPath($OutputDir)
New-Item -ItemType Directory -Path $outputPath -Force | Out-Null

if ([string]::IsNullOrWhiteSpace($WorkingDir)) {
    $workingBase = [IO.Path]::GetTempPath()
} else {
    $workingBase = [IO.Path]::GetFullPath($WorkingDir)
    New-Item -ItemType Directory -Path $workingBase -Force | Out-Null
}

$workRoot = Assert-SafeWorkPath (Join-Path $workingBase ("nubasic-msix-" + [Guid]::NewGuid().ToString('N')))
$layoutPath = Join-Path $workRoot 'layout'
$testCertificate = $null
$testCertificatePath = $null

try {
    New-Item -ItemType Directory -Path $layoutPath -Force | Out-Null

    Get-ChildItem -LiteralPath $stagePath -Force | ForEach-Object {
        Copy-Item -LiteralPath $_.FullName -Destination $layoutPath -Recurse -Force
    }

    # MSI-specific helpers and the VS Code extension are distributed separately.
    # MSIX packages cannot run custom installer actions.
    $installerOnlyFiles = @(
        'bin\diagnose-windows-install.ps1',
        'bin\install-vscode-ext.ps1',
        'bin\nubasic-latest.vsix'
    )
    foreach ($relativePath in $installerOnlyFiles) {
        $candidate = Join-Path $layoutPath $relativePath
        if (Test-Path -LiteralPath $candidate) {
            Remove-Item -LiteralPath $candidate -Force
        }
    }

    Get-ChildItem -LiteralPath $layoutPath -Filter '*.msi' -File |
        ForEach-Object { Remove-Item -LiteralPath $_.FullName -Force }

    $manifestTemplatePath = Join-Path $PSScriptRoot 'AppxManifest.xml.in'
    $manifestPath = Join-Path $layoutPath 'AppxManifest.xml'
    $manifest = [IO.File]::ReadAllText($manifestTemplatePath)
    $manifest = $manifest.Replace('@MSIX_VERSION@', $Version)
    [IO.File]::WriteAllText($manifestPath, $manifest, [Text.UTF8Encoding]::new($false))

    $assetSource = Join-Path $PSScriptRoot 'Assets'
    $assetTarget = Join-Path $layoutPath 'Assets'
    Copy-Item -LiteralPath $assetSource -Destination $assetTarget -Recurse -Force

    $licenseSource = Join-Path $PSScriptRoot '..\..\COPYING'
    Copy-Item -LiteralPath $licenseSource -Destination (Join-Path $layoutPath 'COPYING') -Force

    # Parsing here gives a clearer error than MakeAppx for malformed templates.
    [xml]$manifest | Out-Null

    $makeAppx = Get-WindowsSdkTool 'MakeAppx.exe'
    $packagePath = Join-Path $outputPath ("nuBASIC_{0}_x64.msix" -f $Version)
    if (Test-Path -LiteralPath $packagePath) {
        Remove-Item -LiteralPath $packagePath -Force
    }

    & $makeAppx pack /d $layoutPath /p $packagePath /o
    if ($LASTEXITCODE -ne 0) {
        throw "MakeAppx failed with exit code $LASTEXITCODE"
    }

    if ($SignForTesting) {
        $testCertificate = New-SelfSignedCertificate `
            -Type Custom `
            -Subject $packagePublisher `
            -KeyUsage DigitalSignature `
            -KeyAlgorithm RSA `
            -KeyLength 2048 `
            -HashAlgorithm SHA256 `
            -CertStoreLocation 'Cert:\CurrentUser\My' `
            -NotAfter (Get-Date).AddDays(7) `
            -TextExtension @('2.5.29.37={text}1.3.6.1.5.5.7.3.3')

        if ($testCertificate.Subject -ne $packagePublisher) {
            throw "Test certificate subject '$($testCertificate.Subject)' does not match '$packagePublisher'."
        }

        $testCertificatePath = Join-Path $outputPath ("nuBASIC_{0}_test.cer" -f $Version)
        Export-Certificate `
            -Cert $testCertificate `
            -FilePath $testCertificatePath `
            -Type CERT `
            -Force | Out-Null

        $signTool = Get-WindowsSdkTool 'SignTool.exe'
        & $signTool sign /fd SHA256 /sha1 $testCertificate.Thumbprint /s My $packagePath
        if ($LASTEXITCODE -ne 0) {
            throw "SignTool failed with exit code $LASTEXITCODE"
        }

        # Verify the CMS signature and signer without requiring the temporary
        # self-signed certificate to be trusted on the build machine.
        Assert-MsixSignature `
            -PackagePath $packagePath `
            -ExpectedThumbprint $testCertificate.Thumbprint
    }

    $hash = Get-FileHash -LiteralPath $packagePath -Algorithm SHA256
    $hashPath = "$packagePath.sha256"
    $hashLine = "{0} *{1}{2}" -f $hash.Hash.ToLowerInvariant(), [IO.Path]::GetFileName($packagePath), [Environment]::NewLine
    [IO.File]::WriteAllText($hashPath, $hashLine, [Text.UTF8Encoding]::new($false))

    [PSCustomObject]@{
        Identity = $packageIdentityName
        Publisher = $packagePublisher
        Version = $Version
        Package = $packagePath
        Sha256 = $hash.Hash.ToLowerInvariant()
        TestSigned = [bool]$SignForTesting
        TestCertificate = $testCertificatePath
    }
}
finally {
    if ($testCertificate) {
        $certificatePath = "Cert:\CurrentUser\My\$($testCertificate.Thumbprint)"
        if (Test-Path -LiteralPath $certificatePath) {
            Remove-Item -LiteralPath $certificatePath -Force
        }
    }

    if (Test-Path -LiteralPath $workRoot) {
        Remove-Item -LiteralPath $workRoot -Recurse -Force
    }
}
