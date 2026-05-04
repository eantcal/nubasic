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
brew install cmake git libffi
```

### Configure and Build

```sh
cd nubasic
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(sysctl -n hw.logicalcpu)
sudo make install   # installs nubasic and nubasicdebug to /usr/local/bin
```

### Uninstall on macOS

```sh
sudo xargs rm -f < build/install_manifest.txt
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

### Windows — MSI installer (WiX) or setup.exe (NSIS)

#### Windows installer prerequisites

| Tool | Where to get it |
| ---- | --------------- |
| Visual Studio 2022 (Desktop C++ workload) | [visualstudio.microsoft.com](https://visualstudio.microsoft.com) |
| WiX Toolset v3.x (`candle.exe` + `light.exe` on PATH) | [wixtoolset.org/releases](https://wixtoolset.org/releases/) |
| Node.js 18 LTS+ *(optional — VS Code extension component only)* | [nodejs.org](https://nodejs.org) |

> **NSIS fallback** — add `-DNUBASIC_INSTALLER=NSIS` to cmake and install [NSIS](https://nsis.sourceforge.io/) instead of WiX.

#### Generate the Visual Studio solution

```bat
cd nubasic
mkdir build-win && cd build-win
cmake -G "Visual Studio 17 2022" -A x64 ..
```

#### Using Visual Studio (recommended)

Open `nuBASIC.sln`, set the configuration to **Release**, then in Solution Explorer:

- Expand the **Installer** folder.
- Right-click **`CreateInstaller`** → **Build**.

The MSI is written to the build directory as `nuBASIC_<version>.msi`. The `PACKAGE` target (also in the Installer folder) runs the standard CPack pipeline without the WiX shortcut-patch step.

The installer includes four optional components selectable at setup time:

| Component | Contents |
| --------- | -------- |
| **nuBASIC Runtime** *(required)* | `nubasic.exe`, `nubasicdebug.exe` |
| **nuBASIC IDE** | `NuBasicIDE.exe` (Scintilla-based IDE) |
| **VS Code Extension** | `nubasic-latest.vsix` (auto-installed into VS Code if found) |
| **Example Programs** | Sample `.bas` source files |

`CreateInstaller` internally calls `cmake/RunCpackInstaller.ps1`, which runs CPack and then `cmake/PatchWixMSI.ps1` to inject Start Menu and Desktop shortcuts into the MSI.

#### Using the command line

```bat
cmake --build . --config Release
cmake --build . --target CreateInstaller --config Release
```

**NSIS variant (setup.exe instead of MSI):**

```bat
cmake -G "Visual Studio 17 2022" -A x64 -DNUBASIC_INSTALLER=NSIS ..
cmake --build . --config Release
cmake --build . --target CreateInstaller --config Release
```

#### Windows install and uninstall

Install silently:

```bat
msiexec /i nuBASIC_2.0.0.msi /quiet /norestart
```

Uninstall silently:

```bat
msiexec /x nuBASIC_2.0.0.msi /quiet
```

---

### Ubuntu / Debian — `.deb` package

#### Using the script (recommended)

The script `create_ubuntu_package.sh` (at the repository root) installs build
dependencies, configures, compiles, and packages nuBASIC in one step:

```sh
# Full build (console interpreter + GTK2 IDE):
./create_ubuntu_package.sh

# Console-only (no GTK2 IDE):
./create_ubuntu_package.sh --console-only

# Custom build directory, 8 parallel jobs:
./create_ubuntu_package.sh --build-dir /tmp/nubasic-deb --jobs 8

# Skip apt-get (dependencies already installed):
./create_ubuntu_package.sh --no-install-deps
```

If GTK2 packages are not available the script falls back automatically to a
console-only build.

#### Manual build

```sh
cd nubasic
mkdir build-deb && cd build-deb
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
make -j$(nproc)
cpack -G DEB -D CPACK_PACKAGING_INSTALL_PREFIX=/usr
```

#### Ubuntu install and uninstall

```sh
# Install:
sudo dpkg -i nuBASIC_2.0.0.deb
sudo apt-get install -f   # resolve any missing runtime dependencies

# Uninstall:
sudo dpkg -r nubasic
```

After installation `nubasic` and `nubasicdebug` are in `/usr/bin`. On a full
build `nubasicide` (the GTK2 IDE) is also installed.

---

### macOS — manual install

macOS has no packaged installer. Build and install as described in
[Building on macOS](#building-on-macos) above.

To uninstall:

```sh
sudo xargs rm -f < build/install_manifest.txt
```

---

← [Interpreter Internals](Interpreter-Internals) | [Home](Home)
