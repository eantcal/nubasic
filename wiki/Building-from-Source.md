# Building from Source

← [Interpreter Internals](Interpreter-Internals) | [Home](Home)

---

nuBASIC is written in C++17 and builds with CMake 3.14 or later.

| Artefact | Platform | Description |
|----------|----------|-------------|
| `nubasic` / `nubasic.exe` | All | Command-line interpreter (CLI) |
| `NuBasicIDE.exe` + `SciLexer.dll` | Windows | Graphical IDE with integrated debugger |
| `nubasicide` | Linux | GTK+2 graphical IDE |

A *console* build — interpreter only, no graphics, no IDE — is also available for embedded and
minimal environments.

---

## Getting the Source

```bash
git clone https://github.com/eantcal/nubasic.git
cd nubasic
```

Or download a release archive from
[github.com/eantcal/nubasic/releases](https://github.com/eantcal/nubasic/releases).

---

## Build Targets Overview

| Target | Type | Platforms | Depends on |
|--------|------|-----------|------------|
| `nuBasicInterpreter` | static library | all | *(root library)* |
| `nuWinConsole` | static library | Windows | GDI (Windows SDK) |
| `nuBasicCLI` (`nubasic.exe`) | executable | Windows | `nuBasicInterpreter`, `nuWinConsole` |
| `nuBasicIDE` (`NuBasicIDE.exe`) | executable | Windows | `nuBasicInterpreter`, `nuWinConsole`, `SciLexer` |
| `SciLexer` | shared library (`.dll`) | Windows | Scintilla 5.5.3 + Lexilla 5.4.3 |
| `nubasic` | executable | Linux/macOS | `nuBasicInterpreter`, X11 |
| `nubasicide` | executable | Linux | `nuBasicInterpreter`, GTK+2, Scintilla (static) |

Scintilla 5.5.3 and Lexilla 5.4.3 are downloaded automatically via CMake `FetchContent` on
first configuration (requires internet access). To use the bundled Scintilla 4 instead, pass
`-DSCINTILLA_LOCAL=ON`.

---

## Building on Windows

### Prerequisites

- **Visual Studio 2022** (Community or above) with the *Desktop development with C++* workload
- **CMake 3.14+** — included with Visual Studio 2022, or installable separately
- **Git** — to clone the repository and for CMake's `FetchContent`
- **WiX Toolset 3.x** *(optional)* — required only to build the MSI installer

### Configure and Build with Visual Studio

Open *Developer Command Prompt for VS 2022*, then:

```bat
cd nubasic
mkdir build && cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

The Release binaries are placed in `build\Release\`:

```
build\Release\
├── NuBasicIDE.exe
├── nubasic.exe
└── SciLexer.dll
```

To open the solution in the Visual Studio IDE instead:

```bat
cmake -G "Visual Studio 17 2022" ..
start nuBASIC.sln
```

### Configure and Build with Ninja (MSVC)

```bat
cd nubasic
mkdir build && cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

> **Note:** Open a *Developer Command Prompt* (which runs `vcvarsall.bat`) before invoking CMake
> to ensure the MSVC toolchain is available on `PATH`.

### Console-only Build on Windows

```bat
cmake -G "Visual Studio 17 2022" -DWITH_WIN_IDE=OFF ..
cmake --build . --config Release
```

---

## Building on Linux

### Prerequisites — Full Build (Interpreter + GTK IDE)

On Debian/Ubuntu:

```sh
sudo apt-get update
sudo apt-get install build-essential cmake git \
    libx11-dev \
    libgtk2.0-dev libglib2.0-dev libpango1.0-dev \
    libatk1.0-dev libcairo2-dev \
    xterm xmessage
```

On Fedora/RHEL/CentOS:

```sh
sudo dnf install gcc-c++ cmake git \
    libX11-devel \
    gtk2-devel glib2-devel pango-devel \
    atk-devel cairo-devel \
    xterm xmessage
```

> `xterm` and `xmessage` are used at runtime by `Shell` and `MsgBox`.
> `aplay` (from `alsa-utils`) is used by `PlaySound`.

### Configure and Build

```sh
cd nubasic
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

Produces:
```
/usr/local/bin/nubasic      # console interpreter
/usr/local/bin/nubasicide   # GTK IDE
```

### Console-only Build on Linux

```sh
cmake .. -DWITH_X11=OFF -DWITH_IDE=OFF
make -j$(nproc)
```

This enables `TINY_NUBASIC_VER` (console build mode), stripping all graphics, sound, and IDE
components. The resulting binary has no external dependencies beyond the C++ runtime.

---

## Building on macOS

macOS support is at the same level as the Linux console build. GTK+2 and X11 features are
not available; `WITH_X11` and `WITH_IDE` both default to `OFF` on macOS.

### Prerequisites

```sh
xcode-select --install
brew install cmake git
```

### Configure and Build

```sh
cd nubasic
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.logicalcpu)
sudo make install
```

---

## Building on iOS with iSH

[iSH](https://ish.app) is an Alpine Linux shell environment for iOS/iPadOS. It supports the
console build.

```sh
apk add g++ make cmake git

git clone https://github.com/eantcal/nubasic.git
cd nubasic
mkdir build && cd build
cmake .. -DWITH_X11=OFF -DWITH_IDE=OFF
make -j2
./nubasic
```

---

## CMake Option Reference

| Option | Default | Platform | Description |
|--------|---------|----------|-------------|
| `WITH_X11` | `ON` | Linux | Enable X11 graphics, sound, and xterm/xmessage integration |
| `WITH_IDE` | `ON` | Linux | Build the GTK+2 IDE (`nubasicide`). Requires `WITH_X11=ON` |
| `WITH_WIN_IDE` | `ON` | Windows | Build the Windows IDE (`NuBasicIDE.exe`) |
| `SCINTILLA_LOCAL` | `OFF` | Windows/Linux IDE | Use the bundled Scintilla 4 instead of downloading Scintilla 5.5.3 |
| `SCINTILLA_VERSION` | `"5.5.3"` | Windows/Linux IDE | Scintilla version to download |
| `LEXILLA_VERSION` | `"5.4.3"` | Windows/Linux IDE | Lexilla version to download |
| `NUBASIC_INSTALLER` | `"WIX"` | Windows | Installer generator: `"WIX"` (MSI) or `"NSIS"` (legacy) |
| `CMAKE_BUILD_TYPE` | `Release` | all | `Release` (optimised) or `Debug` (symbols) |

**Example: debug build using local Scintilla (Windows)**

```bat
cmake -G "Visual Studio 17 2022" -DSCINTILLA_LOCAL=ON ..
cmake --build . --config Debug
```

**Example: minimal Linux build with no external dependencies**

```sh
cmake .. -DWITH_X11=OFF -DWITH_IDE=OFF -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

---

## Creating Installers and Packages

### Windows MSI Installer

Requires WiX Toolset 3.x installed and on `PATH`:

```bat
cmake --build . --config Release
cpack -G WIX -C Release
```

The installer:
- Copies `NuBasicIDE.exe`, `nubasic.exe`, and `SciLexer.dll` to `bin\`
- Installs example `.bas` files to `examples\`
- Creates a `nuBASIC` folder in the Start Menu with shortcuts for the IDE, CLI, and Uninstall
- Writes registry keys so that the IDE can locate the examples at startup
- Registers the `.bas` file extension with `NuBasicIDE.exe`

To use the legacy NSIS installer instead:

```bat
cmake .. -DNUBASIC_INSTALLER=NSIS
cpack -G NSIS -C Release
```

### Linux DEB Package

```sh
cd build
cpack -G DEB
```

Produces a `.deb` package installable with `dpkg -i nubasic-*.deb`.

---

← [Interpreter Internals](Interpreter-Internals) | [Home](Home)
