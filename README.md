# nuBASIC

nuBASIC is a modern, open-source BASIC interpreter written in modern C++, available for Windows, Linux, and macOS. It is designed to be both approachable for beginners and capable enough for real programs.

## Three styles in one language

The same nuBASIC interpreter supports three programming styles, and a single program can mix and match them.

- **Classic BASIC** — numbered lines, immediate-mode REPL, `GoTo` / `GoSub` / `On … GoTo`. Type a statement at the prompt and it runs immediately, exactly like the BASICs of the home-computer era.

  ```basic
  10 For i = 1 To 5
  20    Print i
  30 Next i
  ```

- **Structured BASIC** — `Sub` / `Function` with `ByRef` / `ByVal` parameters, structured control flow (`If/ElIf/Else`, `For`, `While`, `Do…Loop While`, `Select Case`, `Exit For/While/Do/Sub/Function`), `Struct` for composite types, `Const`, an `Include` / `#Include` directive for multi-file programs, and a `Function main(argc, argv())` entry point with command-line arguments.

  ```basic
  Function average(values() As Double, n As Integer) As Double
     Dim total As Double
     For i% = 0 To n - 1
        total = total + values(i%)
     Next i%
     average = total / n
  End Function
  ```

- **Object-oriented BASIC** — full classes with `Class` / `End Class`, instance fields and methods, the `Me` self-reference, `Static` class-level methods, single inheritance via `Inherits`, virtual dispatch (`Overridable` / `Overrides`), explicit base-class dispatch via `MyBase`, three-level access control (`Public` / `Protected` / `Private`), constructors (`Sub New`), and RAII destructors (`Sub Delete`) called automatically at scope exit.

  ```basic
  Class Shape
     Overridable Function Describe$() As String
        Describe$ = "a shape"
     End Function
  End Class

  Class Circle
     Inherits Shape
     Public radius As Double
     Overrides Function Describe$() As String
        Describe$ = MyBase.Describe$() + " (radius " + Str$(Me.radius) + ")"
     End Function
  End Class
  ```

- **External native libraries** — `Declare Function … Lib "kernel32.dll" | "libc.so.6" | "libSystem.B.dylib" (...) As Type` declares an exported function from a native shared library. nuBASIC loads it with `LoadLibraryW` (Windows) or `dlopen` (Linux/macOS) and dispatches the call through libffi, so a script can reach Win32, POSIX libc, or any user-built `.so` / `.dylib` directly. Disabled with `--disable-native-calls` for untrusted code.

  ```basic
  Declare Function GetCurrentProcessId Lib "kernel32.dll" () As DWORD
  Print GetCurrentProcessId()
  ```

The classic, structured and OOP styles are not separate dialects: a single source file can use line numbers and `GoTo` next to a `Class` hierarchy, and call into native APIs from any of them. This makes nuBASIC equally usable as a teaching tool for beginners and as a small scripting / embedding language for real programs.

## Features

- **Structured programming** — `Sub`, `Function`, `For`, `While`, `Do…Loop While`, `If/ElIf/Else`, `Select Case`; `Call` keyword; `ByRef` / `ByVal` parameter passing; open-ended array parameters (`param() As Type`); `Include` / `#Include` for multi-file programs
- **Classes with static methods** — `Class`/`End Class` with instance fields, instance methods (`Me`), and `Static Function`/`Static Sub` callable as `ClassName.Method()` without an instance
- **`main()` entry point** — if a `Function main(...)` is defined, execution starts there; CLI arguments are available via `argc` and `argv() As String`
- **Namespaced modules** — `Syntax Modern` activates qualified names (`math::sin`, `string::left$`); `Using Module` imports a module for unqualified access; `Syntax Legacy` restores classic mode
- **Rich type system** — Integer, Double, Boolean, Long64, String, Byte, Any, user-defined `Struct`
- **Arrays and hash tables** built into the language
- **Full file I/O** — sequential, binary, and random access
- **Graphics** — lines, rectangles, ellipses, filled shapes, text, bitmaps, pixel access (Windows GDI / Linux X11)
- **Screen mode switching** — `Screen 0` (text/headless) / `Screen 1` (GDI graphics), like GW-BASIC `SCREEN`; `-t` CLI flag for CI/scripting
- **Flicker-free rendering** — `ScreenLock` / `ScreenUnlock` / `Refresh` for double-buffered animation
- **Mouse and keyboard input** — `GetMouse()` returns a `Mouse` struct (`x`, `y`, `btn`) in one call
- **Date/time** — `GetDateTime()` returns a `DateTime` struct with all fields in one call
- **UTF-8** string literals and console output
- **Hex literals** — classic BASIC `&hFF` / `&HFF` syntax, with optional `0xFF` / `0XFF` accepted for imported C-style constants
- **Optimized runtime values** — scalar `variant_t` values are stored inline, struct/object metadata is boxed, and struct payload copies use copy-on-write
- **Built-in help** — `Help <keyword>` and `Apropos <topic>` accessible from the REPL
- **Native library calls** — `Declare Function name Lib "kernel32.dll" \| "libc.so.6" \| "libSystem.B.dylib" (...) As Type` plus runtime memory helpers (`NativeAlloc`, `NativePoke*`, `NativePeekStr$`); libffi-backed, opt-out via `--disable-native-calls`
- **Native IDE** for Windows and Linux (GTK+2) with Scintilla-based syntax highlighting, auto-completion, code folding, bookmarks; integrated debugger with breakpoints, Step Into / Step Over / Step Out / Run to Cursor / Pause-Break, watch and call stack
- **Multi-source projects** — `.nbp` project files (entry point, syntax mode, display name) work across the IDE, the CLI, and the VS Code debugger
- **VS Code extension** — syntax highlighting + debug adapter (breakpoints, step modes, watch, call stack), backed by `nubasicdebug` as a console-subsystem debug backend; auto-installed by the Windows MSI's optional `VSCodeExtension` component
- **Console build** for headless/embedded systems (no graphics, no external dependencies)
- **MIT License**

## Documentation

- **[Wiki](https://github.com/eantcal/nubasic/wiki)** — full language reference, graphics API, IDE guide, interpreter internals, and build instructions
- **[User Guide](https://github.com/eantcal/nubasic/blob/main/docs/nubasic-guide.md)** — complete guide in a single document (v2.0)
- **[Making nuBASIC 2.0](https://github.com/eantcal/nubasic/blob/main/docs/making-nubasic-2.0.md)** — developer-oriented internals note about debugger, runtime values, native calls, and performance work ([Italiano](https://github.com/eantcal/nubasic/blob/main/docs/making-nubasic-2.0-it.md))
- **[Examples](https://github.com/eantcal/nubasic/tree/main/examples)** — ready-to-run `.bas` programs covering games, graphics, fractals, animations, and more

## Quick Start

```bash
# Interactive REPL
nubasic

# Run a program file
nubasic myprogram.bas
```

```basic
Print "Hello, world!"
```

## Building and Installing

nuBASIC builds with CMake 3.14+ and a C++20 compiler (`gcc`/`clang` on POSIX, MSVC 2022 on Windows).  
See the [Building from Source](https://github.com/eantcal/nubasic/wiki/Building-from-Source) wiki page for the full CMake option reference.

---

### Windows — MSI installer (WiX) or setup.exe (NSIS)

#### Prerequisites

| Tool | Where to get it |
| ---- | --------------- |
| Visual Studio 2022 (Desktop C++ workload) | [visualstudio.microsoft.com](https://visualstudio.microsoft.com) |
| CMake 3.14+ | bundled with VS 2022, or [cmake.org](https://cmake.org) |
| WiX Toolset v3.x (`candle.exe` + `light.exe` on PATH) | [wixtoolset.org/releases](https://wixtoolset.org/releases/) |
| Node.js 18 LTS+ (optional — VS Code extension only) | see [VS Code extension](#vs-code-extension--nodejs-dependency) |

> **NSIS fallback** — if WiX is not available, add `-DNUBASIC_INSTALLER=NSIS` to the cmake command and install [NSIS](https://nsis.sourceforge.io/) (`makensis.exe` on PATH) instead.

#### Generate the Visual Studio solution

```bat
git clone https://github.com/eantcal/nubasic.git
cd nubasic
mkdir build-win && cd build-win
cmake -G "Visual Studio 17 2022" -A x64 ..
```

#### Build and create the installer from Visual Studio (recommended)

After generating the solution, open `nuBASIC.sln` in Visual Studio 2022.

1. Set the configuration to **Release** (top toolbar drop-down).
2. In **Solution Explorer**, expand the **Installer** folder.
3. Right-click **`CreateInstaller`** → **Build**.

The MSI is written to the build directory as `nuBASIC_<version>.msi`.  
The `PACKAGE` target (also in the **Installer** folder) is an alternative that runs the standard CPack pipeline without the WiX shortcut patch.

> The installer includes four optional components selectable during setup:
>
> | Component | Contents |
> | --------- | -------- |
> | **nuBASIC Runtime** *(required)* | `nubasic.exe`, `nubasicdebug.exe` |
> | **nuBASIC IDE** | `NuBasicIDE.exe` (Scintilla-based IDE) |
> | **VS Code Extension** | `nubasic-latest.vsix` (auto-installed if VS Code is found) |
> | **Example Programs** | Sample `.bas` source files |

#### Build and create the installer from the command line

```powershell
# From the build-win directory created above:
cmake --build . --config Release
cmake --build . --target CreateInstaller --config Release
```

The `CreateInstaller` target internally calls `cmake/RunCpackInstaller.ps1`, which runs CPack and then `cmake/PatchWixMSI.ps1` to inject Start Menu and Desktop shortcuts into the MSI.

**NSIS variant (setup.exe instead of MSI):**

```powershell
cmake -G "Visual Studio 17 2022" -A x64 -DNUBASIC_INSTALLER=NSIS ..
cmake --build . --config Release
cmake --build . --target CreateInstaller --config Release
```

#### Install

Double-click the generated `nuBASIC_<version>.msi` and follow the wizard, or install silently:

```powershell
msiexec /i nuBASIC_2.0.0.msi /quiet /norestart
```

To uninstall:

```powershell
msiexec /x nuBASIC_2.0.0.msi /quiet
```

---

### Ubuntu / Debian — `.deb` package

The script `create_ubuntu_package.sh` installs build dependencies, configures, compiles, and packages nuBASIC in one step.

#### Ubuntu prerequisites

| Tool | Installed automatically by the script |
| ---- | ------------------------------------- |
| `build-essential`, `cmake`, `ninja-build`, `pkg-config`, `libffi-dev`, `dpkg-dev` | yes (via `apt-get`) |
| `libgtk2.0-dev` and related GTK2 libraries (IDE only) | yes, if GTK2 is available |
| `sudo` access | required |

#### Usage

```sh
# Full build (console interpreter + GTK2 IDE):
./create_ubuntu_package.sh

# Console-only (no GTK2 IDE):
./create_ubuntu_package.sh --console-only

# Custom build directory, 8 parallel jobs:
./create_ubuntu_package.sh --build-dir /tmp/nubasic-deb --jobs 8

# Skip apt-get (dependencies already installed):
./create_ubuntu_package.sh --no-install-deps

# Help:
./create_ubuntu_package.sh --help
```

The script prints the path of the generated `.deb` on success.

If GTK2 packages are not available in the configured apt sources, the script automatically falls back to a console-only build.

#### Manual build (without the script)

```sh
git clone https://github.com/eantcal/nubasic.git
cd nubasic
mkdir build-deb && cd build-deb
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
make -j$(nproc)
cpack -G DEB -D CPACK_PACKAGING_INSTALL_PREFIX=/usr
```

#### Ubuntu install

```sh
# Inspect package contents and metadata:
dpkg-deb -I nuBASIC_2.0.0.deb

# Install:
sudo dpkg -i nuBASIC_2.0.0.deb

# Resolve any missing runtime dependencies:
sudo apt-get install -f
```

After installation, `nubasic` and `nubasicdebug` are in `/usr/bin`.  
On full builds, `nubasicide` (the GTK2 IDE) is also installed.

---

### macOS — build and install manually

macOS builds the console interpreter and debug backend only (no IDE).

#### macOS prerequisites

```sh
# Xcode command-line tools (provides clang and make):
xcode-select --install

# CMake via Homebrew:
brew install cmake

# libffi (required for native library calls):
brew install libffi
```

#### macOS build and install

```sh
git clone https://github.com/eantcal/nubasic.git
cd nubasic
mkdir build-mac && cd build-mac
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(sysctl -n hw.logicalcpu)
sudo make install   # installs nubasic and nubasicdebug to /usr/local/bin
```

#### Uninstall

```sh
sudo xargs rm -f < build-mac/install_manifest.txt
```

---

## VS Code extension — Node.js dependency

The VS Code extension (`vscode-nubasic/`) is built from TypeScript and provides syntax highlighting, a debug adapter (breakpoints, step modes, watch, call stack), and project support for `.nbp` files.

CMake automatically skips the extension targets if `node` or `npm` are not found on `PATH`, emitting a warning at configure time. To enable them, install **Node.js 18 LTS** or later.

### Installing Node.js

**Ubuntu / Debian:**

```sh
# NodeSource repository — installs Node.js 20 LTS
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt-get install -y nodejs
```

**macOS:**

```sh
brew install node
```

**Windows:**

```powershell
winget install OpenJS.NodeJS.LTS
```

### Building the extension manually

```sh
cd vscode-nubasic
npm install           # install TypeScript and type definitions
npm run compile       # compile src/extension.ts → out/extension.js
bash package-vsix.sh  # create nubasic-<version>.vsix
```

On Windows, use `package-vsix.ps1` in place of `package-vsix.sh`.

### Installing the extension

```sh
code --install-extension vscode-nubasic/nubasic-latest.vsix --force
```

Or via CMake after the extension has been built:

```sh
cmake --build <build-dir> --target VscodeExtensionInstall
```

On Windows the MSI installer can optionally install and activate the extension automatically if VS Code is detected on the system.

## Running the Test Suite

Tests live in the `tests/` directory as `test_*.bas` files.  They use
`Screen 0` (text mode) so output goes to stdout without needing a GUI.

**Windows (PowerShell):**

```powershell
.\tests\run_tests.ps1 -Interpreter .\build\release\Release\nubasic.exe
```

**From Visual Studio:** right-click the **RunTests** target → Build.

**Linux / macOS (Bash):**

```sh
./tests/run_tests.sh --interpreter ./build/release/nubasic
```

Or via CMake:

```sh
cmake --build build/release --target RunTests
```

## Platforms

| Platform | Interpreter | IDE |
| -------- | ----------- | --- |
| Windows | `nubasic.exe` (console subsystem) · `nubasicgdi.exe` (GDI console for graphics) · `nubasicdebug.exe` (VS Code debug backend) | `NuBasicIDE.exe` (Scintilla editor) |
| Linux | `nubasic` · `nubasicdebug` | `nubasicide` (GTK+2 + Scintilla editor) |
| macOS | `nubasic` · `nubasicdebug` | — |
| iOS (iSH) | `nubasic` (console build) | — |

## License

nuBASIC is open source under the [MIT License](https://opensource.org/licenses/MIT).

Bug reports and feature requests: [antonino.calderone@gmail.com](mailto:antonino.calderone@gmail.com)

<img width="2878" height="1700" alt="image" src="https://github.com/user-attachments/assets/9a4c9c54-b3c9-492e-99ef-e03a7f2b6cab" />

<html><br><br></html>

![nuBASIC IDE on Linux/GTK](https://user-images.githubusercontent.com/13032534/27808819-69aaa2c2-6042-11e7-9132-675d1c71d162.png)

<img width="2876" height="1704" alt="image" src="https://github.com/user-attachments/assets/dc0f0f01-33da-4df6-9ad3-33d4f43b92b8" />

