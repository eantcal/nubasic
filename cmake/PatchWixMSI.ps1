#Requires -Version 5.0
<#
.SYNOPSIS
    Patches CPack-generated WiX WXS files to add Start Menu shortcuts,
    a desktop shortcut, registry keys, and .bas file association.
    Then recompiles and relinks the MSI.

.DESCRIPTION
    CPack's WiX generator garbage-collects Fragment sections whose
    declared symbols are not directly reachable from the main product
    tree.  The only reliable workaround is to inject content into the
    CPack-generated directories.wxs and features.wxs files *after*
    CPack produces them and *before* candle/light finalise the MSI.

    Called by the CreateInstaller CMake target after a successful
    CPack run.

.PARAMETER WixPkgDir
    Path to the _CPack_Packages/win64/WIX staging directory.

.PARAMETER Candle
    Full path to candle.exe (WiX Toolset v3).

.PARAMETER Light
    Full path to light.exe (WiX Toolset v3).

.PARAMETER MsiOutput
    Destination path for the final MSI.
#>
param(
    [Parameter(Mandatory)] [string] $WixPkgDir,
    [Parameter(Mandatory)] [string] $Candle,
    [Parameter(Mandatory)] [string] $Light,
    [Parameter(Mandatory)] [string] $MsiOutput
)

$ErrorActionPreference = 'Stop'
$WixNs = 'http://schemas.microsoft.com/wix/2006/wi'

function Add-WixChild([xml]$Doc, [System.Xml.XmlElement]$Parent, [string]$XmlString) {
    # Parse the fragment string as its own document, then import the root
    # node into $Doc (preserving namespace) and append it to $Parent.
    $frag = [xml]$XmlString
    $imported = $Doc.ImportNode($frag.DocumentElement, $true)
    [void]$Parent.AppendChild($imported)
}

# ---------------------------------------------------------------------------
# 1. Patch directories.wxs — add ProgramMenuFolder and DesktopFolder
#    as children of TARGETDIR.
# ---------------------------------------------------------------------------
$dirFile = Join-Path $WixPkgDir 'directories.wxs'
$dirDoc  = [xml](Get-Content $dirFile -Encoding UTF8)
$nsmgr   = New-Object System.Xml.XmlNamespaceManager($dirDoc.NameTable)
$nsmgr.AddNamespace('wix', $WixNs)

if (-not $dirDoc.SelectSingleNode('//wix:Directory[@Id="NuBASIC_MenuDir"]', $nsmgr)) {
    $targetDir = $dirDoc.SelectSingleNode('//wix:Directory[@Id="TARGETDIR"]', $nsmgr)

    Add-WixChild $dirDoc $targetDir @'
<Directory xmlns="http://schemas.microsoft.com/wix/2006/wi"
           Id="ProgramMenuFolder">
    <Directory Id="NuBASIC_MenuDir" Name="nuBASIC">
        <Component Id="CM_COMP_StartMenuShortcuts"
                   Guid="{B3D7E2A1-4F8C-4A9B-8E2F-1A2B3C4D5E6F}"
                   Win64="yes">
            <CreateFolder/>
            <Shortcut Id="NuBASIC_IDE_Link"
                      Name="nuBASIC IDE"
                      Description="nuBASIC Integrated Development Environment"
                      Target="[INSTALL_ROOT]bin\NuBasicIDE.exe"
                      WorkingDirectory="INSTALL_ROOT"/>
            <Shortcut Id="NuBASIC_CLI_Link"
                      Name="nuBASIC Console"
                      Description="nuBASIC Command-Line Interpreter"
                      Target="[INSTALL_ROOT]bin\nubasic.exe"
                      WorkingDirectory="INSTALL_ROOT"/>
            <Shortcut Id="NuBASIC_Uninstall_Link"
                      Name="Uninstall nuBASIC"
                      Target="[SystemFolder]msiexec.exe"
                      Arguments="/x [ProductCode]"/>
            <RemoveFolder Id="NuBASIC_MenuDir_Remove"
                          Directory="NuBASIC_MenuDir"
                          On="uninstall"/>
            <RegistryValue Root="HKCU"
                           Key="Software\nuBASIC"
                           Name="StartMenuShortcuts"
                           Type="integer"
                           Value="1"
                           KeyPath="yes"/>
        </Component>
    </Directory>
</Directory>
'@

    Add-WixChild $dirDoc $targetDir @'
<Directory xmlns="http://schemas.microsoft.com/wix/2006/wi"
           Id="DesktopFolder">
    <Component Id="CM_COMP_DesktopShortcut"
               Guid="{E4A5B6C7-8901-4DEF-0123-456789ABCDEF}"
               Win64="yes">
        <Shortcut Id="NuBASIC_Desktop_Link"
                  Name="nuBASIC IDE"
                  Description="nuBASIC Integrated Development Environment"
                  Target="[INSTALL_ROOT]bin\NuBasicIDE.exe"
                  WorkingDirectory="INSTALL_ROOT"/>
        <RegistryValue Root="HKCU"
                       Key="Software\nuBASIC"
                       Name="DesktopShortcut"
                       Type="integer"
                       Value="1"
                       KeyPath="yes"/>
    </Component>
</Directory>
'@

    $dirDoc.Save($dirFile)
    Write-Host "Patched: $dirFile"
} else {
    Write-Host "Already patched, skipping: $dirFile"
}

# ---------------------------------------------------------------------------
# 2. Patch features.wxs — add ComponentRefs to the existing FeatureRef and
#    append a new Fragment with the registry / file-association components.
# ---------------------------------------------------------------------------
$featFile = Join-Path $WixPkgDir 'features.wxs'
$featDoc  = [xml](Get-Content $featFile -Encoding UTF8)
$nsmgr2   = New-Object System.Xml.XmlNamespaceManager($featDoc.NameTable)
$nsmgr2.AddNamespace('wix', $WixNs)

if (-not $featDoc.SelectSingleNode('//wix:ComponentRef[@Id="CM_COMP_StartMenuShortcuts"]', $nsmgr2)) {
    # Add ComponentRefs to existing FeatureRef
    $featureRef = $featDoc.SelectSingleNode('//wix:FeatureRef[@Id="ProductFeature"]', $nsmgr2)
    foreach ($id in 'CM_COMP_StartMenuShortcuts','CM_COMP_DesktopShortcut',
                     'CM_COMP_RegistryKeys','CM_COMP_BASFileAssoc') {
        $node = $featDoc.CreateElement('ComponentRef', $WixNs)
        $node.SetAttribute('Id', $id)
        [void]$featureRef.AppendChild($node)
    }

    # Append new Fragment with registry and .bas file-assoc components
    Add-WixChild $featDoc $featDoc.DocumentElement @'
<Fragment xmlns="http://schemas.microsoft.com/wix/2006/wi">
    <DirectoryRef Id="INSTALL_ROOT">
        <Component Id="CM_COMP_RegistryKeys"
                   Guid="{C2E3F4A5-6789-4BCD-EF01-234567890ABC}"
                   Win64="yes">
            <RegistryValue Root="HKLM"
                           Key="Software\nuBASIC"
                           Name="InstallDir"
                           Type="string"
                           Value="[INSTALL_ROOT]"
                           KeyPath="yes"/>
            <RegistryValue Root="HKLM"
                           Key="Software\nuBASIC"
                           Name="ExamplesDir"
                           Type="string"
                           Value="[INSTALL_ROOT]examples\"/>
        </Component>
        <Component Id="CM_COMP_BASFileAssoc"
                   Guid="{D3F4A5B6-7890-4CDE-F012-34567890BCDE}"
                   Win64="yes">
            <RegistryValue Root="HKCU"
                           Key="Software\Classes\.bas"
                           Type="string"
                           Value="nuBASICFile"
                           KeyPath="yes"/>
            <RegistryValue Root="HKCU"
                           Key="Software\Classes\nuBASICFile"
                           Type="string"
                           Value="nuBASIC Source File"/>
            <RegistryValue Root="HKCU"
                           Key="Software\Classes\nuBASICFile\DefaultIcon"
                           Type="string"
                           Value="[INSTALL_ROOT]bin\NuBasicIDE.exe,0"/>
            <RegistryValue Root="HKCU"
                           Key="Software\Classes\nuBASICFile\shell\open\command"
                           Type="string"
                           Value="&quot;[INSTALL_ROOT]bin\NuBasicIDE.exe&quot; &quot;%1&quot;"/>
        </Component>
    </DirectoryRef>
</Fragment>
'@

    $featDoc.Save($featFile)
    Write-Host "Patched: $featFile"
} else {
    Write-Host "Already patched, skipping: $featFile"
}

# ---------------------------------------------------------------------------
# 3. Recompile patched WXS files
# ---------------------------------------------------------------------------
Write-Host "Recompiling directories.wxs..."
& $Candle -nologo -arch x64 -out "$WixPkgDir\directories.wixobj" $dirFile
if ($LASTEXITCODE -ne 0) { throw "candle.exe failed on directories.wxs" }

Write-Host "Recompiling features.wxs..."
& $Candle -nologo -arch x64 -out "$WixPkgDir\features.wixobj" $featFile
if ($LASTEXITCODE -ne 0) { throw "candle.exe failed on features.wxs" }

# ---------------------------------------------------------------------------
# 4. Relink — omit WixPatch.wixobj (all content is now inlined)
# ---------------------------------------------------------------------------
Write-Host "Relinking MSI -> $MsiOutput ..."
& $Light -nologo -out $MsiOutput -ext WixUIExtension `
    "$WixPkgDir\directories.wixobj" `
    "$WixPkgDir\files.wixobj"       `
    "$WixPkgDir\features.wixobj"    `
    "$WixPkgDir\main.wixobj"
if ($LASTEXITCODE -ne 0) { throw "light.exe failed" }

Write-Host "Done: $MsiOutput"
