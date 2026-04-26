# nuBASIC Executable Launch Matrix

This note makes the Windows launch policy explicit so `nubasic.exe`,
`nubasicdebug.exe`, and `nubasicgdi.exe` do not drift apart.

## Binaries

- `nubasic.exe`
  - Text CLI for normal command-line execution.
  - Inherits the caller's stdio.
  - Used for batch mode, tests, and non-debug editor runs.
  - On Windows it may delegate to `nubasicgdi.exe` for graphics programs,
    unless `-t` / `--text-mode` is explicitly present.

- `nubasicdebug.exe`
  - Text CLI dedicated to machine-controlled debugging.
  - Inherits the caller's stdio and speaks the machine interface used by
    VS Code and debugger contract tests.
  - Never intended as the normal interactive graphics entry point.

- `nubasicgdi.exe`
  - Windows GDI frontend.
  - Used for graphics-oriented interactive execution on Windows.
  - Owns the windowed/GDI experience and message loop.

## Launch policy by workflow

- VS Code `Run nuBASIC File`
  - File: [extension.ts](C:\repo\nubasic\vscode-nubasic\src\extension.ts)
  - Launches `nubasic.exe`
  - Purpose: normal editor run

- VS Code `Debug nuBASIC File`
  - File: [extension.ts](C:\repo\nubasic\vscode-nubasic\src\extension.ts)
  - Launches `nubasicdebug.exe` when available, otherwise falls back to the
    configured base executable
  - Purpose: debugger backend

- `RunTests`
  - File: [CMakeLists.txt](C:\repo\nubasic\CMakeLists.txt)
  - Uses target `nuBasicCLI` -> `nubasic.exe`
  - The test runner passes `-t`
  - Therefore `nubasic.exe` must stay headless and must not delegate to GDI
    when `-t` is present

- `RunDebuggerContract`
  - File: [CMakeLists.txt](C:\repo\nubasic\CMakeLists.txt)
  - Uses target `nuBasicDebugCLI` -> `nubasicdebug.exe`

- `RunInterruptibleInputContract`
  - File: [CMakeLists.txt](C:\repo\nubasic\CMakeLists.txt)
  - Uses target `nuBasicDebugCLI` -> `nubasicdebug.exe`

- `RunInterruptibleInputStrContract`
  - File: [CMakeLists.txt](C:\repo\nubasic\CMakeLists.txt)
  - Uses target `nuBasicDebugCLI` -> `nubasicdebug.exe`

- `RunRosettaBenchmark`
  - File: [CMakeLists.txt](C:\repo\nubasic\CMakeLists.txt)
  - Uses target `nuBasicCLI` -> `nubasic.exe`
  - The benchmark runner passes `-t`

- `CreateInstaller`
  - File: [CMakeLists.txt](C:\repo\nubasic\CMakeLists.txt)
  - Builds/packages artifacts only
  - Does not run the interpreter as part of normal packaging

- MSI custom action `install-vscode-ext.ps1`
  - File: [install-vscode-ext.ps1](C:\repo\nubasic\cmake\install-vscode-ext.ps1)
  - Installs the VSIX into VS Code
  - Does not launch `nubasic.exe`, `nubasicdebug.exe`, or `nubasicgdi.exe`

- Developer reinstall target `ReinstallNuBasicDev`
  - File: [reinstall-nubasic-dev.ps1](C:\repo\nubasic\cmake\reinstall-nubasic-dev.ps1)
  - Reinstalls MSI + VSIX
  - Does not choose a runtime interpreter; it only refreshes installed artifacts

## Windows routing rule

In [main.cc](C:\repo\nubasic\cli\main.cc):

- `nubasic.exe` may auto-route to `nubasicgdi.exe` only when:
  - an `-e <file>` program is being executed
  - the source is detected as graphics-using
  - `-t` / `--text-mode` is **not** present

- `nubasic.exe` must remain text/headless when:
  - `-t` / `--text-mode` is present
  - tests or CI invoke it
  - a caller depends on redirected stdio

## Practical rule of thumb

- Want normal command-line execution: use `nubasic.exe`
- Want VS Code/debugger integration: use `nubasicdebug.exe`
- Want Windows graphics UI execution: use `nubasicgdi.exe`
