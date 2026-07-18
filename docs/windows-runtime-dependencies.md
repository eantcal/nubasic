# Windows Runtime DLL Dependencies

This page documents the DLL dependency closure for the Windows build of
nuBASIC. It is intended as a packaging checklist for MSI releases and as a
support reference when an installed executable does not start on a clean
Windows 10/11 machine.

The list below was checked from the Release binaries with:

```powershell
 dumpbin /DEPENDENTS <binary>
```

## Packaged DLLs

These DLLs are not guaranteed to exist on every end-user machine and should be
installed next to the nuBASIC executables in `bin`.

| DLL | Source | Expected MSI location | Why it matters |
|---|---|---|---|
| `ffi-8.dll` or `ffi.dll` | vcpkg `libffi:x64-windows` | `Core\bin` | Required by native DLL calls. Because the interpreter links to libffi, missing this DLL can prevent `nubasic.exe`, `nubasicdebug.exe`, `nubasicgdi.exe`, and `NuBasicIDE.exe` from starting. |
| `vcruntime140.dll` | Microsoft Visual C++ Redistributable / `InstallRequiredSystemLibraries` | `Core\bin` | MSVC C runtime. |
| `vcruntime140_1.dll` | Microsoft Visual C++ Redistributable / `InstallRequiredSystemLibraries` | `Core\bin` | MSVC C++ exception/runtime support. |
| `msvcp140.dll` | Microsoft Visual C++ Redistributable / `InstallRequiredSystemLibraries` | `Core\bin` | MSVC C++ standard library. |
| `msvcp140_1.dll` | Microsoft Visual C++ Redistributable / `InstallRequiredSystemLibraries` | `Core\bin` | Additional MSVC C++ runtime support. |
| `msvcp140_2.dll` | Microsoft Visual C++ Redistributable / `InstallRequiredSystemLibraries` | `Core\bin` | Additional MSVC C++ runtime support. |
| `msvcp140_atomic_wait.dll` | Microsoft Visual C++ Redistributable / `InstallRequiredSystemLibraries` | `Core\bin` | Used by newer MSVC standard library implementations. |
| `msvcp140_codecvt_ids.dll` | Microsoft Visual C++ Redistributable / `InstallRequiredSystemLibraries` | `Core\bin` | Used by newer MSVC standard library implementations. |
| `concrt140.dll` | Microsoft Visual C++ Redistributable / `InstallRequiredSystemLibraries` | `Core\bin` | Concurrency runtime, pulled in by MSVC runtime dependency collection. |
| `SciLexer.dll` | Built from Scintilla/Lexilla | `IDE\bin` | Required by `NuBasicIDE.exe`; if missing, the IDE can fail shortly after the splash screen. |

For the 2.0.2 MSI staging tree, the Core component contains:

```text
concrt140.dll
ffi-8.dll
msvcp140.dll
msvcp140_1.dll
msvcp140_2.dll
msvcp140_atomic_wait.dll
msvcp140_codecvt_ids.dll
vcruntime140.dll
vcruntime140_1.dll
```

The IDE component contains:

```text
NuBasicIDE.exe
SciLexer.dll
```

## System DLLs

These imports are provided by Windows on supported systems. nuBASIC 2.x targets
Windows 10 and newer; Windows 7 is not supported.

| DLL family | Expected provider |
|---|---|
| `KERNEL32.dll`, `USER32.dll`, `GDI32.dll`, `SHELL32.dll`, `ADVAPI32.dll` | Windows system directories |
| `COMCTL32.dll`, `COMDLG32.dll`, `ole32.dll`, `OLEAUT32.dll`, `IMM32.dll` | Windows system directories |
| `gdiplus.dll`, `WINMM.dll` | Windows system directories |
| `api-ms-win-crt-*.dll` | Windows Universal C Runtime, present on Windows 10/11 |

If an `api-ms-win-crt-*` DLL is reported missing, the machine is either below
the supported OS baseline, damaged, or missing Windows servicing updates. Do
not bundle those API-set DLLs manually.

## Binary Import Summary

### `nubasic.exe`

```text
ADVAPI32.dll
api-ms-win-crt-conio-l1-1-0.dll
api-ms-win-crt-convert-l1-1-0.dll
api-ms-win-crt-environment-l1-1-0.dll
api-ms-win-crt-filesystem-l1-1-0.dll
api-ms-win-crt-heap-l1-1-0.dll
api-ms-win-crt-locale-l1-1-0.dll
api-ms-win-crt-math-l1-1-0.dll
api-ms-win-crt-runtime-l1-1-0.dll
api-ms-win-crt-stdio-l1-1-0.dll
api-ms-win-crt-string-l1-1-0.dll
api-ms-win-crt-time-l1-1-0.dll
api-ms-win-crt-utility-l1-1-0.dll
ffi-8.dll
GDI32.dll
gdiplus.dll
KERNEL32.dll
MSVCP140.dll
ole32.dll
USER32.dll
VCRUNTIME140.dll
VCRUNTIME140_1.dll
WINMM.dll
```

### `nubasicdebug.exe`

Same dependency set as `nubasic.exe`.

### `nubasicgdi.exe`

Same dependency set as `nubasic.exe`, plus:

```text
SHELL32.dll
```

### `NuBasicIDE.exe`

```text
ADVAPI32.dll
api-ms-win-crt-conio-l1-1-0.dll
api-ms-win-crt-convert-l1-1-0.dll
api-ms-win-crt-environment-l1-1-0.dll
api-ms-win-crt-filesystem-l1-1-0.dll
api-ms-win-crt-heap-l1-1-0.dll
api-ms-win-crt-locale-l1-1-0.dll
api-ms-win-crt-math-l1-1-0.dll
api-ms-win-crt-runtime-l1-1-0.dll
api-ms-win-crt-stdio-l1-1-0.dll
api-ms-win-crt-string-l1-1-0.dll
api-ms-win-crt-time-l1-1-0.dll
api-ms-win-crt-utility-l1-1-0.dll
COMCTL32.dll
COMDLG32.dll
ffi-8.dll
GDI32.dll
gdiplus.dll
KERNEL32.dll
MSVCP140.dll
ole32.dll
SHELL32.dll
USER32.dll
VCRUNTIME140.dll
VCRUNTIME140_1.dll
WINMM.dll
```

`NuBasicIDE.exe` also dynamically loads `SciLexer.dll` at runtime.

### `SciLexer.dll`

```text
ADVAPI32.dll
api-ms-win-crt-convert-l1-1-0.dll
api-ms-win-crt-heap-l1-1-0.dll
api-ms-win-crt-locale-l1-1-0.dll
api-ms-win-crt-math-l1-1-0.dll
api-ms-win-crt-runtime-l1-1-0.dll
api-ms-win-crt-stdio-l1-1-0.dll
api-ms-win-crt-string-l1-1-0.dll
GDI32.dll
IMM32.dll
KERNEL32.dll
MSVCP140.dll
ole32.dll
OLEAUT32.dll
USER32.dll
VCRUNTIME140.dll
VCRUNTIME140_1.dll
```

## Diagnostic Procedure

On an affected Windows machine, run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass `
  -File "C:\Program Files\nuBASIC\bin\diagnose-windows-install.ps1"
```

The generated zip contains `installed-bin.txt` and `smoke-tests.txt`, which are
the first files to inspect for missing DLL problems.
