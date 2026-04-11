# nuBASIC

nuBASIC is a modern, open-source BASIC interpreter written in modern C++, available for Windows, Linux, and macOS. It is designed to be both approachable for beginners and capable enough for real programs.

## Features

- **Structured programming** — `Sub`, `Function`, `For`, `While`, `Do…Loop While`, `If/ElIf/Else`; `Call` keyword; `ByRef` / `ByVal` parameter passing; `Include` / `#Include` for multi-file programs
- **Rich type system** — Integer, Double, Boolean, Long64, String, Byte, Any, user-defined `Struct`
- **Arrays and hash tables** built into the language
- **Full file I/O** — sequential, binary, and random access
- **Graphics** — lines, rectangles, ellipses, filled shapes, text, bitmaps, pixel access (Windows GDI / Linux X11)
- **Screen mode switching** — `Screen 0` (text/headless) / `Screen 1` (GDI graphics), like GW-BASIC `SCREEN`; `-t` CLI flag for CI/scripting
- **Flicker-free rendering** — `ScreenLock` / `ScreenUnlock` / `Refresh` for double-buffered animation
- **Mouse and keyboard input** — `GetMouse()` returns a `Mouse` struct (`x`, `y`, `btn`) in one call
- **Date/time** — `GetDateTime()` returns a `DateTime` struct with all fields in one call
- **UTF-8** string literals and console output
- **Built-in help** — `Help <keyword>` and `Apropos <topic>` accessible from the REPL
- **IDE** for Windows and Linux (GTK+2) with syntax highlighting, auto-completion, and integrated debugger
- **Console build** for headless/embedded systems (no graphics, no external dependencies)
- **MIT License**

## Documentation

- **[Wiki](https://github.com/eantcal/nubasic/wiki)** — full language reference, graphics API, IDE guide, interpreter internals, and build instructions
- **[User Guide](https://github.com/eantcal/nubasic/blob/master/docs/nubasic-guide.md)** — complete guide in a single document (v2.0)
- **[Examples](https://github.com/eantcal/nubasic/tree/master/examples)** — ready-to-run `.bas` programs covering games, graphics, fractals, animations, and more

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

## Building from Source

nuBASIC builds with CMake 3.14+ and a C++20 compiler.

**Windows (Visual Studio 2022):**

```bat
git clone https://github.com/eantcal/nubasic.git
cd nubasic
mkdir build && cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

**Linux:**

```sh
git clone https://github.com/eantcal/nubasic.git
cd nubasic
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

**Console build (no graphics, no IDE):**

```sh
cmake .. -DWITH_X11=OFF -DWITH_IDE=OFF
make -j$(nproc)
```

See the [Building from Source](https://github.com/eantcal/nubasic/wiki/Building-from-Source) wiki page for full details including macOS, iOS/iSH, CMake options, and installer creation.

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
|----------|-------------|-----|
| Windows | `nubasic.exe` | `NuBasicIDE.exe` (GDI console + Scintilla editor) |
| Linux | `nubasic` | `nubasicide` (GTK+2 + Scintilla editor) |
| macOS | `nubasic` | — |
| iOS (iSH) | `nubasic` (console build) | — |

## License

nuBASIC is open source under the [MIT License](https://opensource.org/licenses/MIT).

Bug reports and feature requests: antonino.calderone@gmail.com

<img width="1080" height="983" alt="nuBASIC IDE for Windows" src="https://github.com/user-attachments/assets/b2f1e0cf-8304-44e3-b6dc-e0f9b421a48f" />

<html><br><br></html>

![nuBASIC IDE on Linux/GTK](https://user-images.githubusercontent.com/13032534/27808819-69aaa2c2-6042-11e7-9132-675d1c71d162.png)

