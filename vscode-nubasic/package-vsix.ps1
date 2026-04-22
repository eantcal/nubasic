#Requires -Version 5.0

$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$pkg = Get-Content (Join-Path $root 'package.json') -Raw | ConvertFrom-Json
$version = $pkg.version

Add-Type -AssemblyName System.IO.Compression.FileSystem

$stage = Join-Path $root ('.vsix-stage-' + [guid]::NewGuid().ToString('N'))
$vsixOut = Join-Path $root "nubasic-$version.vsix"

if (Test-Path $vsixOut) {
    Remove-Item -LiteralPath $vsixOut -Force
}

$extensionDir = Join-Path $stage 'extension'
New-Item -ItemType Directory -Path $extensionDir | Out-Null
New-Item -ItemType Directory -Path (Join-Path $extensionDir 'out') | Out-Null
New-Item -ItemType Directory -Path (Join-Path $extensionDir 'syntaxes') | Out-Null

Copy-Item -LiteralPath (Join-Path $root 'language-configuration.json') -Destination $extensionDir
Copy-Item -LiteralPath (Join-Path $root 'LICENSE') -Destination (Join-Path $extensionDir 'LICENSE.txt')
Copy-Item -LiteralPath (Join-Path $root 'package.json') -Destination $extensionDir
Copy-Item -LiteralPath (Join-Path $root 'out\extension.js') -Destination (Join-Path $extensionDir 'out')
Copy-Item -LiteralPath (Join-Path $root 'syntaxes\nubasic.tmLanguage.json') -Destination (Join-Path $extensionDir 'syntaxes')

$manifest = @"
<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011" xmlns:d="http://schemas.microsoft.com/developer/vsx-schema-design/2011">
  <Metadata>
    <Identity Language="en-US" Id="$($pkg.name)" Version="$version" Publisher="$($pkg.publisher)" />
    <DisplayName>$($pkg.displayName)</DisplayName>
    <Description xml:space="preserve">$($pkg.description)</Description>
    <Tags>keybindings,debuggers,nubasic,nuBASIC,__ext_bas</Tags>
    <Categories>Programming Languages</Categories>
    <GalleryFlags>Public</GalleryFlags>
    <Properties>
      <Property Id="Microsoft.VisualStudio.Code.Engine" Value="$($pkg.engines.vscode)" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionDependencies" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionPack" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionKind" Value="workspace" />
      <Property Id="Microsoft.VisualStudio.Code.LocalizedLanguages" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.EnabledApiProposals" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.ExecutesCode" Value="true" />
      <Property Id="Microsoft.VisualStudio.Services.Links.Source" Value="$($pkg.repository.url).git" />
      <Property Id="Microsoft.VisualStudio.Services.Links.Getstarted" Value="$($pkg.repository.url).git" />
      <Property Id="Microsoft.VisualStudio.Services.Links.GitHub" Value="$($pkg.repository.url).git" />
      <Property Id="Microsoft.VisualStudio.Services.Links.Support" Value="$($pkg.repository.url)/issues" />
      <Property Id="Microsoft.VisualStudio.Services.Links.Learn" Value="$($pkg.repository.url)#readme" />
      <Property Id="Microsoft.VisualStudio.Services.GitHubFlavoredMarkdown" Value="true" />
      <Property Id="Microsoft.VisualStudio.Services.Content.Pricing" Value="Free" />
    </Properties>
    <License>extension/LICENSE.txt</License>
  </Metadata>
  <Installation>
    <InstallationTarget Id="Microsoft.VisualStudio.Code" />
  </Installation>
  <Dependencies />
  <Assets>
    <Asset Type="Microsoft.VisualStudio.Code.Manifest" Path="extension/package.json" Addressable="true" />
    <Asset Type="Microsoft.VisualStudio.Services.Content.License" Path="extension/LICENSE.txt" Addressable="true" />
  </Assets>
</PackageManifest>
"@

$contentTypes = @"
<?xml version="1.0" encoding="utf-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension=".js" ContentType="application/javascript" />
  <Default Extension=".json" ContentType="application/json" />
  <Default Extension=".txt" ContentType="text/plain" />
  <Default Extension=".vsixmanifest" ContentType="text/xml" />
</Types>
"@

Set-Content -LiteralPath (Join-Path $stage 'extension.vsixmanifest') -Value $manifest -Encoding UTF8
Set-Content -LiteralPath (Join-Path $stage '[Content_Types].xml') -Value $contentTypes -Encoding UTF8

[System.IO.Compression.ZipFile]::CreateFromDirectory(
    $stage,
    $vsixOut,
    [System.IO.Compression.CompressionLevel]::Optimal,
    $false
)
try {
    Remove-Item -LiteralPath $stage -Recurse -Force -ErrorAction Stop
}
catch {
    Write-Warning "Could not remove temporary VSIX stage '$stage': $_"
}
