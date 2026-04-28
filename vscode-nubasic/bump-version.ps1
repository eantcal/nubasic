#Requires -Version 5.0

$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
node (Join-Path $root 'bump-version.js')
