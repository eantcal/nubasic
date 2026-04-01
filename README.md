# nuBASIC

nuBASIC is a modern, open-source BASIC interpreter written in C++17, available for Windows, Linux, and macOS. It is designed to be both approachable for beginners and capable enough for real programs.

![nuBASIC IDE on Linux/GTK](https://user-images.githubusercontent.com/13032534/27808819-69aaa2c2-6042-11e7-9132-675d1c71d162.png)

## Features

- **Structured programming** — `Sub`, `Function`, `For`, `While`, `Do…Loop While`, `If/ElIf/Else`
- **Rich type system** — Integer, Double, Boolean, Long64, String, Byte, Any, user-defined `Struct`
- **Arrays and hash tables** built into the language
- **Full file I/O** — sequential, binary, and random access
- **Graphics** — lines, rectangles, ellipses, filled shapes, text, bitmaps, pixel access (Windows GDI / Linux X11)
- **Flicker-free rendering** — `ScreenLock` / `ScreenUnlock` / `Refresh` for double-buffered animation
- **Mouse and keyboard input**
- **UTF-8** string literals and console output
- **Built-in help** — `Help <keyword>` and `Apropos <topic>` accessible from the REPL
- **IDE** for Windows and Linux (GTK+2) with syntax highlighting, auto-completion, and integrated debugger
- **Tiny build** for headless/embedded systems (no graphics, no external dependencies)
- **MIT License**

## Documentation

- **[Wiki](https://github.com/eantcal/nubasic/wiki)** — full language reference, graphics API, IDE guide, interpreter internals, and build instructions
- **[User Guide](https://github.com/eantcal/nubasic/blob/master/docs/nubasic-guide.md)** — complete guide in a single document (v1.61)
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

nuBASIC builds with CMake 3.14+ and a C++17 compiler.

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

**Tiny build (no graphics, no IDE):**

```sh
cmake .. -DWITH_X11=OFF -DWITH_IDE=OFF
make -j$(nproc)
```

See the [Building from Source](https://github.com/eantcal/nubasic/wiki/Building-from-Source) wiki page for full details including macOS, iOS/iSH, CMake options, and installer creation.

## Platforms

| Platform | Interpreter | IDE |
|----------|-------------|-----|
| Windows | `nubasic.exe` | `NuBasicIDE.exe` (GDI console + Scintilla editor) |
| Linux | `nubasic` | `nubasicide` (GTK+2 + Scintilla editor) |
| macOS | `nubasic` | — |
| iOS (iSH) | `nubasic` (tiny) | — |

## License

nuBASIC is open source under the [MIT License](https://opensource.org/licenses/MIT).

Bug reports and feature requests: antonino.calderone@gmail.com
