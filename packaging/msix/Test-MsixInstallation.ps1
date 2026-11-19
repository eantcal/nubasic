[CmdletBinding()]
param(
    [string] $BuildDir = '',

    [string] $StageDir = '',

    [string] $OutputDir = '',

    [string] $Version = '',

    [switch] $Reinstall,

    [switch] $SkipVCLibs,

    [switch] $LaunchIDE,

    [switch] $RunWack,

    [switch] $Cleanup
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$packageIdentityName = 'AntoninoCalderone.nuBASIC'
$packagePublisher = 'CN=006FC507-ECB1-41CF-9858-405FEB4A81E5'
$vclibsName = 'Microsoft.VCLibs.140.00.UWPDesktop'
$vclibsMinimumVersion = [version]'14.0.24217.0'
$vclibsUri = 'https://aka.ms/Microsoft.VCLibs.x64.14.00.Desktop.appx'

function Assert-Administrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]::new($identity)
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        throw 'Run this script from an elevated PowerShell window (Run as administrator).'
    }
}

function Resolve-FullPath {
    param(
        [Parameter(Mandatory = $true)][string] $Path,
        [Parameter(Mandatory = $true)][string] $BasePath
    )

    if ([IO.Path]::IsPathRooted($Path)) {
        return [IO.Path]::GetFullPath($Path)
    }

    return [IO.Path]::GetFullPath((Join-Path $BasePath $Path))
}

function Get-NuBasicVersion {
    param([Parameter(Mandatory = $true)][string] $CMakeListsPath)

    $cmake = [IO.File]::ReadAllText($CMakeListsPath)
    $major = [regex]::Match($cmake, 'set\(MAJOR_VERSION\s+([0-9]+)\)').Groups[1].Value
    $minor = [regex]::Match($cmake, 'set\(MINOR_VERSION\s+([0-9]+)\)').Groups[1].Value
    $patch = [regex]::Match($cmake, 'set\(PATCH_VERSION\s+([0-9]+)\)').Groups[1].Value

    if (-not $major -or -not $minor -or -not $patch) {
        throw "Could not determine the nuBASIC version from $CMakeListsPath."
    }

    return "$major.$minor.$patch.0"
}

function Invoke-CheckedCommand {
    param(
        [Parameter(Mandatory = $true)][string] $FilePath,
        [Parameter(Mandatory = $true)][string[]] $Arguments,
        [Parameter(Mandatory = $true)][string] $Description
    )

    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Description failed with exit code $LASTEXITCODE."
    }
}

function Test-ManifestRegistration {
    param([Parameter(Mandatory = $true)][string] $InstallLocation)

    $manifestPath = Join-Path $InstallLocation 'AppxManifest.xml'
    if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) {
        throw "The registered AppxManifest.xml was not found: $manifestPath"
    }

    $manifest = [IO.File]::ReadAllText($manifestPath)
    $requiredManifestValues = @(
        'windows.appExecutionAlias',
        'Alias="nubasic.exe"',
        '<uap:FileType>.bas</uap:FileType>',
        '<uap:FileType>.nbp</uap:FileType>',
        'Name="runFullTrust"'
    )

    foreach ($requiredValue in $requiredManifestValues) {
        if (-not $manifest.Contains($requiredValue)) {
            throw "The registered manifest does not contain '$requiredValue'."
        }
    }
}

function Get-WackTool {
    $programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
    $candidate = Join-Path $programFilesX86 'Windows Kits\10\App Certification Kit\appcert.exe'
    if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
        throw "Windows App Certification Kit was not found: $candidate"
    }

    return $candidate
}

Assert-Administrator

$repoRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$cmakeListsPath = Join-Path $repoRoot 'CMakeLists.txt'
$testRoot = Join-Path $repoRoot '_generated\msix-install-test'
$runId = '{0}-{1}' -f (Get-Date -Format 'yyyyMMdd-HHmmss'), [Guid]::NewGuid().ToString('N').Substring(0, 8)
$runRoot = Join-Path $testRoot $runId

if ([string]::IsNullOrWhiteSpace($Version)) {
    $Version = Get-NuBasicVersion -CMakeListsPath $cmakeListsPath
}

if ($Version -notmatch '^[1-9][0-9]{0,4}\.[0-9]{1,5}\.[0-9]{1,5}\.0$') {
    throw "Invalid Microsoft Store package version: $Version"
}

if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $repoRoot 'build\release'
} else {
    $BuildDir = Resolve-FullPath -Path $BuildDir -BasePath $repoRoot
}

if ([string]::IsNullOrWhiteSpace($StageDir)) {
    $StageDir = Join-Path $runRoot 'stage'
} else {
    $StageDir = Resolve-FullPath -Path $StageDir -BasePath $repoRoot
}

if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $OutputDir = Join-Path $runRoot 'output'
} else {
    $OutputDir = Resolve-FullPath -Path $OutputDir -BasePath $repoRoot
}

$workingDir = Join-Path $runRoot 'work'
New-Item -ItemType Directory -Path $runRoot, $OutputDir, $workingDir -Force | Out-Null

$certificateImportedByScript = $false
$certificateThumbprint = $null
$installedByScript = $false
$testSucceeded = $false
$packagePath = $null
$testCertificatePath = $null

try {
    if (-not $PSBoundParameters.ContainsKey('StageDir')) {
        if (-not (Test-Path -LiteralPath $BuildDir -PathType Container)) {
            throw "The build directory does not exist: $BuildDir"
        }

        New-Item -ItemType Directory -Path $StageDir -Force | Out-Null
        Write-Host "Staging the Release build in $StageDir ..."
        Invoke-CheckedCommand `
            -FilePath 'cmake' `
            -Arguments @('--install', $BuildDir, '--config', 'Release', '--prefix', $StageDir) `
            -Description 'CMake install staging'
    } elseif (-not (Test-Path -LiteralPath $StageDir -PathType Container)) {
        throw "The supplied staging directory does not exist: $StageDir"
    }

    Write-Host "Building a signed test MSIX for version $Version ..."
    $packagingOutput = & (Join-Path $PSScriptRoot 'New-MsixPackage.ps1') `
        -StageDir $StageDir `
        -OutputDir $OutputDir `
        -WorkingDir $workingDir `
        -Version $Version `
        -SignForTesting

    $packageInfo = $packagingOutput |
        Where-Object { $_ -and $_.PSObject.Properties['Package'] } |
        Select-Object -Last 1

    if (-not $packageInfo) {
        throw 'The packaging script did not return package information.'
    }

    $packagePath = [string]$packageInfo.Package
    $testCertificatePath = [string]$packageInfo.TestCertificate
    if (-not (Test-Path -LiteralPath $packagePath -PathType Leaf)) {
        throw "The test MSIX was not created: $packagePath"
    }
    if (-not (Test-Path -LiteralPath $testCertificatePath -PathType Leaf)) {
        throw "The public test certificate was not created: $testCertificatePath"
    }

    $certificate = [Security.Cryptography.X509Certificates.X509Certificate2]::new($testCertificatePath)
    $certificateThumbprint = $certificate.Thumbprint
    if ($certificate.Subject -ne $packagePublisher) {
        throw "The test certificate subject '$($certificate.Subject)' does not match '$packagePublisher'."
    }

    $trustedCertificatePath = "Cert:\LocalMachine\TrustedPeople\$certificateThumbprint"
    if (-not (Test-Path -LiteralPath $trustedCertificatePath)) {
        Write-Host "Trusting test certificate $certificateThumbprint for this machine ..."
        Import-Certificate `
            -FilePath $testCertificatePath `
            -CertStoreLocation 'Cert:\LocalMachine\TrustedPeople' | Out-Null
        $certificateImportedByScript = $true
    } else {
        Write-Host 'The test certificate is already trusted on this machine.'
    }

    $signature = Get-AuthenticodeSignature -FilePath $packagePath
    if ($signature.Status -ne 'Valid') {
        throw "The MSIX signature is not valid after trusting the certificate: $($signature.StatusMessage)"
    }

    if (-not $SkipVCLibs) {
        $vclibs = Get-AppxPackage -AllUsers -Name $vclibsName -ErrorAction SilentlyContinue |
            Where-Object { [version]$_.Version -ge $vclibsMinimumVersion } |
            Sort-Object { [version]$_.Version } -Descending |
            Select-Object -First 1

        if (-not $vclibs) {
            $vclibsPath = Join-Path $runRoot 'Microsoft.VCLibs.x64.14.00.Desktop.appx'
            Write-Host "Downloading the x64 VCLibs framework from $vclibsUri ..."
            Invoke-WebRequest -Uri $vclibsUri -OutFile $vclibsPath -UseBasicParsing

            $vclibsSignature = Get-AuthenticodeSignature -FilePath $vclibsPath
            if ($vclibsSignature.Status -ne 'Valid') {
                throw "The downloaded VCLibs package signature is not valid: $($vclibsSignature.StatusMessage)"
            }

            Write-Host 'Installing the x64 VCLibs framework ...'
            Add-AppxPackage -Path $vclibsPath
        } else {
            Write-Host "VCLibs $($vclibs.Version) is already installed."
        }
    }

    $existingPackage = Get-AppxPackage -Name $packageIdentityName -ErrorAction SilentlyContinue
    if ($existingPackage) {
        if (-not $Reinstall) {
            throw "nuBASIC is already installed as '$($existingPackage.PackageFullName)'. Use -Reinstall to replace it."
        }

        Write-Host "Removing existing package $($existingPackage.PackageFullName) ..."
        $existingPackage | Remove-AppxPackage
    }

    Write-Host "Installing $packagePath ..."
    Add-AppxPackage -Path $packagePath
    $installedByScript = $true

    $installedPackage = Get-AppxPackage -Name $packageIdentityName -ErrorAction Stop
    if ([version]$installedPackage.Version -ne [version]$Version) {
        throw "Installed version '$($installedPackage.Version)' does not match '$Version'."
    }
    if ($installedPackage.Publisher -ne $packagePublisher) {
        throw "Installed publisher '$($installedPackage.Publisher)' does not match '$packagePublisher'."
    }
    if ($installedPackage.Status -ne 'Ok') {
        throw "The installed package status is '$($installedPackage.Status)'."
    }

    $installedCli = Join-Path $installedPackage.InstallLocation 'bin\nubasic.exe'
    $installedIde = Join-Path $installedPackage.InstallLocation 'bin\NuBasicIDE.exe'
    foreach ($requiredExecutable in @($installedCli, $installedIde)) {
        if (-not (Test-Path -LiteralPath $requiredExecutable -PathType Leaf)) {
            throw "Installed executable is missing: $requiredExecutable"
        }
    }

    Test-ManifestRegistration -InstallLocation $installedPackage.InstallLocation

    Write-Host 'Running the packaged interpreter smoke test ...'
    Invoke-CheckedCommand `
        -FilePath $installedCli `
        -Arguments @('-v') `
        -Description 'Packaged nuBASIC interpreter smoke test'

    $executionAlias = Join-Path $env:LOCALAPPDATA 'Microsoft\WindowsApps\nubasic.exe'
    if (-not (Test-Path -LiteralPath $executionAlias)) {
        throw "The nubasic.exe execution alias was not registered: $executionAlias"
    }

    Write-Host 'Running the app execution alias smoke test ...'
    Invoke-CheckedCommand `
        -FilePath $executionAlias `
        -Arguments @('-v') `
        -Description 'nuBASIC app execution alias smoke test'

    if ($LaunchIDE) {
        $applicationUserModelId = "$($installedPackage.PackageFamilyName)!NuBasicIDE"
        Write-Host "Launching nuBASIC IDE through $applicationUserModelId ..."
        Start-Process -FilePath 'explorer.exe' -ArgumentList "shell:AppsFolder\$applicationUserModelId"
    }

    $wackReportPath = $null
    if ($RunWack) {
        $appCert = Get-WackTool
        $wackReportPath = Join-Path $OutputDir 'WindowsAppCertificationKitReport.xml'
        Write-Host 'Running Windows App Certification Kit ...'
        Invoke-CheckedCommand -FilePath $appCert -Arguments @('reset') -Description 'WACK reset'
        Invoke-CheckedCommand `
            -FilePath $appCert `
            -Arguments @(
                'test',
                '-packagefullname', $installedPackage.PackageFullName,
                '-reportoutputpath', $wackReportPath
            ) `
            -Description 'Windows App Certification Kit'
    }

    Write-Host ''
    Write-Host 'nuBASIC MSIX installation test passed.' -ForegroundColor Green
    $testSucceeded = $true
    [PSCustomObject]@{
        Package = $packagePath
        PackageFullName = $installedPackage.PackageFullName
        PackageFamilyName = $installedPackage.PackageFamilyName
        InstallLocation = $installedPackage.InstallLocation
        Version = [string]$installedPackage.Version
        Publisher = $installedPackage.Publisher
        CertificateThumbprint = $certificateThumbprint
        WackReport = $wackReportPath
        CleanupRequested = [bool]$Cleanup
    }
}
finally {
    $removeTestState = $Cleanup -or -not $testSucceeded

    if ($removeTestState -and $installedByScript) {
        $installedPackage = Get-AppxPackage -Name $packageIdentityName -ErrorAction SilentlyContinue
        if ($installedPackage) {
            Write-Host "Removing test installation $($installedPackage.PackageFullName) ..."
            $installedPackage | Remove-AppxPackage
        }
    }

    if ($removeTestState -and $certificateImportedByScript -and $certificateThumbprint) {
        $trustedCertificatePath = "Cert:\LocalMachine\TrustedPeople\$certificateThumbprint"
        if (Test-Path -LiteralPath $trustedCertificatePath) {
            Write-Host "Removing test certificate $certificateThumbprint from Trusted People ..."
            Remove-Item -LiteralPath $trustedCertificatePath -Force
        }
    }

    if ($testSucceeded -and -not $Cleanup -and $certificateImportedByScript -and $certificateThumbprint) {
        Write-Warning "The package and test certificate remain installed. Re-run with -Reinstall -Cleanup after testing, or remove package '$packageIdentityName' and certificate '$certificateThumbprint' manually."
    }
}
