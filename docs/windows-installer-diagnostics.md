# Windows Installer Diagnostics

Use this checklist when the MSI completes but an installed nuBASIC executable
does not start on Windows 10/11. This symptom usually means a runtime DLL was
not installed next to the executables, or Windows policy blocked the executable.

## Capture the install log

Ask the affected user to install with verbose logging:

```powershell
msiexec /i .\nuBASIC_2.0.2.msi /L*V "$env:TEMP\nubasic-install.log"
```

The useful failure marker is often `Return value 3`; the real cause is usually
20-50 lines above it.

## Collect the installed-machine diagnostics

The MSI installs a read-only collector in the `bin` directory:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File "C:\Program Files\nuBASIC\bin\diagnose-windows-install.ps1" `
  -MsiLog "$env:TEMP\nubasic-install.log"
```

If nuBASIC was installed somewhere else:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File "D:\Tools\nuBASIC\bin\diagnose-windows-install.ps1" `
  -InstallDir "D:\Tools\nuBASIC" `
  -MsiLog "$env:TEMP\nubasic-install.log"
```

The script creates `%TEMP%\nubasic-install-diagnostics-YYYYMMDD-HHMMSS.zip`.

## What to check first

Open `installed-bin.txt` and verify that the expected runtime DLLs are present. The full dependency list is documented in [`windows-runtime-dependencies.md`](windows-runtime-dependencies.md):

- `ffi-8.dll` or `ffi.dll`
- `vcruntime140.dll`
- `vcruntime140_1.dll`
- `msvcp140.dll`
- `msvcp140_1.dll`
- `msvcp140_2.dll`
- `concrt140.dll`
- `SciLexer.dll` if the IDE component was selected

Open `smoke-tests.txt`. Missing DLLs commonly surface as process failures before
nuBASIC prints its version banner.

Open `registry.txt` to verify the install directory and uninstall registration.
Open `msi-events.txt` for Windows Installer events recorded by the Application
event log.

## Why this matters

A developer workstation may have vcpkg or Visual Studio runtime DLLs on `PATH`,
so a package can appear to work locally even if the MSI forgot to include one.
A clean Windows 11 machine is a better approximation of a real end-user host:
the executable must be able to start using only system DLLs and files installed
under `C:\Program Files\nuBASIC\bin`.
