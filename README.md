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
- **Built-in help** — `Help <keyword>` and `Apropos <topic>` accessible from the REPL
- **Native library calls** — `Declare Function name Lib "kernel32.dll" \| "libc.so.6" \| "libSystem.B.dylib" (...) As Type` plus runtime memory helpers (`NativeAlloc`, `NativePoke*`, `NativePeekStr$`); libffi-backed, opt-out via `--disable-native-calls`
- **Native IDE** for Windows and Linux (GTK+2) with Scintilla-based syntax highlighting, auto-completion, code folding, bookmarks; integrated debugger with breakpoints, Step Into / Step Over / Step Out / Run to Cursor / Pause-Break, watch and call stack
- **Multi-source projects** — `.nbp` project files (entry point, syntax mode, display name) work across the IDE, the CLI, and the VS Code debugger
- **VS Code extension** — syntax highlighting + debug adapter (breakpoints, step modes, watch, call stack), backed by `nubasicdebug` as a console-subsystem debug backend; auto-installed by the Windows MSI's optional `VSCodeExtension` component
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
| Windows | `nubasic.exe` (console subsystem) · `nubasicgdi.exe` (GDI console for graphics) · `nubasicdebug.exe` (VS Code debug backend) | `NuBasicIDE.exe` (Scintilla editor) |
| Linux | `nubasic` · `nubasicdebug` | `nubasicide` (GTK+2 + Scintilla editor) |
| macOS | `nubasic` · `nubasicdebug` | — |
| iOS (iSH) | `nubasic` (console build) | — |

## License

nuBASIC is open source under the [MIT License](https://opensource.org/licenses/MIT).

Bug reports and feature requests: antonino.calderone@gmail.com

<img width="1080" height="983" alt="nuBASIC IDE for Windows" src="https://github.com/user-attachments/assets/b2f1e0cf-8304-44e3-b6dc-e0f9b421a48f" />

<html><br><br></html>

![nuBASIC IDE on Linux/GTK](https://user-images.githubusercontent.com/13032534/27808819-69aaa2c2-6042-11e7-9132-675d1c71d162.png)

<img width="2876" height="1704" alt="image" src="https://github.com/user-attachments/assets/dc0f0f01-33da-4df6-9ad3-33d4f43b92b8" />

