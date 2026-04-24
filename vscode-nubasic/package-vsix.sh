#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"

read_pkg() { node -p "require('${ROOT}/package.json').$1"; }

VERSION="$(read_pkg version)"
PKG_NAME="$(read_pkg name)"
PUBLISHER="$(read_pkg publisher)"
DISPLAY_NAME="$(read_pkg displayName)"
DESCRIPTION="$(read_pkg description)"
ENGINE_VSCODE="$(read_pkg 'engines.vscode')"
REPO_URL="$(read_pkg 'repository.url')"

VSIX_OUT="${ROOT}/nubasic-${VERSION}.vsix"
STAGE="$(mktemp -d "${TMPDIR:-/tmp}/.vsix-stage-XXXXXX")"
trap 'rm -rf "$STAGE"' EXIT

EXT_DIR="${STAGE}/extension"
mkdir -p "${EXT_DIR}/out" "${EXT_DIR}/syntaxes"

cp "${ROOT}/language-configuration.json"          "${EXT_DIR}/"
cp "${ROOT}/LICENSE"                               "${EXT_DIR}/LICENSE.txt"
cp "${ROOT}/package.json"                          "${EXT_DIR}/"
cp "${ROOT}/out/extension.js"                      "${EXT_DIR}/out/"
cp "${ROOT}/syntaxes/nubasic.tmLanguage.json"      "${EXT_DIR}/syntaxes/"

cat > "${STAGE}/extension.vsixmanifest" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011" xmlns:d="http://schemas.microsoft.com/developer/vsx-schema-design/2011">
  <Metadata>
    <Identity Language="en-US" Id="${PKG_NAME}" Version="${VERSION}" Publisher="${PUBLISHER}" />
    <DisplayName>${DISPLAY_NAME}</DisplayName>
    <Description xml:space="preserve">${DESCRIPTION}</Description>
    <Tags>keybindings,debuggers,nubasic,nuBASIC,__ext_bas</Tags>
    <Categories>Programming Languages</Categories>
    <GalleryFlags>Public</GalleryFlags>
    <Properties>
      <Property Id="Microsoft.VisualStudio.Code.Engine" Value="${ENGINE_VSCODE}" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionDependencies" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionPack" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.ExtensionKind" Value="workspace" />
      <Property Id="Microsoft.VisualStudio.Code.LocalizedLanguages" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.EnabledApiProposals" Value="" />
      <Property Id="Microsoft.VisualStudio.Code.ExecutesCode" Value="true" />
      <Property Id="Microsoft.VisualStudio.Services.Links.Source" Value="${REPO_URL}.git" />
      <Property Id="Microsoft.VisualStudio.Services.Links.Getstarted" Value="${REPO_URL}.git" />
      <Property Id="Microsoft.VisualStudio.Services.Links.GitHub" Value="${REPO_URL}.git" />
      <Property Id="Microsoft.VisualStudio.Services.Links.Support" Value="${REPO_URL}/issues" />
      <Property Id="Microsoft.VisualStudio.Services.Links.Learn" Value="${REPO_URL}#readme" />
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
EOF

cat > "${STAGE}/[Content_Types].xml" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension=".js" ContentType="application/javascript" />
  <Default Extension=".json" ContentType="application/json" />
  <Default Extension=".txt" ContentType="text/plain" />
  <Default Extension=".vsixmanifest" ContentType="text/xml" />
</Types>
EOF

rm -f "${VSIX_OUT}"
(cd "${STAGE}" && zip -r "${VSIX_OUT}" . -x "*.DS_Store")

echo "Created: ${VSIX_OUT}"
