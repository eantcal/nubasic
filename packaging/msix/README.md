# Microsoft Store MSIX packaging

This directory builds the Windows x64 Store package for nuBASIC. The package
identity is reserved in Microsoft Partner Center:

| Field | Value |
| --- | --- |
| Package identity name | `AntoninoCalderone.nuBASIC` |
| Publisher | `CN=006FC507-ECB1-41CF-9858-405FEB4A81E5` |
| Publisher display name | `Antonino Calderone` |
| Package family name | `AntoninoCalderone.nuBASIC_mcsr0jr7vxhd6` |
| Store ID | `9NH7SQ3S54J0` |

The package contains the Windows IDE, command-line interpreter, runtime
libraries, and examples. It declares:

- the `nubasic.exe` command-line execution alias;
- `.bas` and `.nbp` file associations with the IDE;
- the Microsoft Visual C++ Desktop Runtime framework dependency;
- the `runFullTrust` capability required by the existing Win32 application.

The VS Code extension and MSI diagnostic/install helpers are intentionally not
included. MSIX cannot run the MSI custom actions that install those components;
the extension remains available through its independent distribution channel.

## Build locally

First configure and build nuBASIC, then stage the CMake install tree:

```powershell
cmake --install _generated\build\release --config Release --prefix _generated\msix-stage
```

Create a Store-compatible package. The fourth version component must remain
zero because it is reserved for Microsoft Store use. The Store workflow leaves
the package unsigned; Microsoft signs it after certification:

```powershell
.\packaging\msix\New-MsixPackage.ps1 `
  -StageDir _generated\msix-stage `
  -OutputDir _generated\msix-output `
  -Version 2.0.5.0
```

For a locally installable development build, add `-SignForTesting`. This creates
a short-lived self-signed code-signing certificate whose subject matches the
Partner Center publisher, signs and verifies the package, exports the public
`.cer` beside it, then removes the private certificate. Import that `.cer` into
the local machine **Trusted People** certificate store from an elevated shell
before installing the package. Do not distribute the certificate or
test-signed package as a public release.

For local sideload testing, install the x64
`Microsoft.VCLibs.140.00.UWPDesktop` framework package first if it is not
already present. Microsoft Store installs that dependency automatically for
customers.

## Test installation automatically

Run the installation test from an elevated PowerShell window. The script
stages the existing Release build, creates and verifies a test-signed MSIX,
trusts its public certificate in the local machine `Trusted People` store,
installs VCLibs when needed, installs nuBASIC, and tests both the packaged CLI
and the `nubasic.exe` execution alias:

```powershell
.\packaging\msix\Test-MsixInstallation.ps1
```

Useful options are:

- `-LaunchIDE` to activate the installed IDE after the automated checks;
- `-RunWack` to run Windows App Certification Kit when it is installed;
- `-Reinstall` to replace an existing nuBASIC MSIX installation;
- `-Cleanup` to remove the test package and the certificate imported by the
  script after the checks;
- `-StageDir <path>` to test an already staged Windows artifact without running
  `cmake --install`;
- `-SkipVCLibs` when the framework dependency is managed separately.

For a disposable validation run that leaves no test package or certificate
installed:

```powershell
.\packaging\msix\Test-MsixInstallation.ps1 -Cleanup
```

## Store submission

Upload the unsigned `.msix` produced by the Windows GitHub Actions workflow in
Partner Center under **nuBASIC > Product release > Packages**. Microsoft Store
re-signs MSIX packages after certification, so a commercial certificate is not
needed for this distribution channel. Before submission, test on a clean
Windows machine and run the Windows App Certification Kit. Store listing text,
screenshots, age rating, privacy policy, and pricing/availability are managed
separately in Partner Center.

The manifest declares the restricted `runFullTrust` capability because nuBASIC
is an existing Win32 interpreter and IDE. A suitable Partner Center
justification is: “nuBASIC needs full-trust desktop execution to open and save
user-selected BASIC source and project files, run the console and graphics
interpreters, and load the local assets and libraries referenced by those
programs.”
