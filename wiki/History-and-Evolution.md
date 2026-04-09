# History and Evolution of nuBASIC

← [IDE](IDE) | Next: [Interpreter Internals](Interpreter-Internals)

---

## Origins (March 2014)

nuBASIC was created by Antonino Calderone and first published in March 2014, starting from
version 0.1. The initial implementation established the core of what would become a complete
language: an expression interpreter, variable support, and a set of fundamental math functions
(`Sin`, `Cos`, `ASin`, `ACos`, and the other standard trigonometric and transcendental functions).

Within the same month, the main BASIC statements appeared: `Let`, `Print`, `Input`, `GoTo`,
`If/Then/Else`, `For/To/Step`, and the essential interactive commands `Run`, `List`, `New`,
`Clr`. An I/O console was implemented for both Windows and Linux, making nuBASIC immediately
usable on both platforms. `.deb` packages for Ubuntu/Debian and RPM packages for Fedora and
openSUSE followed shortly after.

## Growing the Language (April – June 2014)

Spring 2014 brought a remarkable rate of expansion. Each release added features that
transformed nuBASIC from a toy interpreter into a usable programming tool:

- **File operations** — `Open`, `Close`, `Input#`, `Print#`, `FOpen`, `Seek`, `FTell`, `FSize`,
  and byte arrays (`@` suffix), enabling programs to read and write persistent data (v0.15)
- **Subroutines and functions** — `Sub`/`End Sub`, `Function`/`End Function`, `Exit Sub`,
  `Exit Function`, with proper call stacks and local variable scopes (v0.18, v0.19)
- **Structured loops** — `While`/`Wend` and `Do`/`Loop While`, reducing reliance on `GoTo` (v0.16)
- **Label support** — `GoTo` and `GoSub` could now target named labels as well as line numbers (v0.14)
- **Breakpoints and tracing** — conditional breakpoints (`Break If`), program tracing (`TrOn`/
  `TrOff`), and step-mode execution (`StOn`/`StOff`) for debugging (v0.13–v0.20)
- **First graphics** — `Line`, `Rect`, `FillRect`, `Ellipse`, `FillEllipse` on Windows (GDI)
  and Linux (X11), then `TextOut` and `SetPixel` (v0.7–v0.9)
- **Mouse support** — `GetMouseX()`, `GetMouseY()`, `GetMouseBtn()` (v1.06)
- **`Eval` function** — evaluate a nuBASIC expression from a string at run time (v1.10)
- First game examples: `breakout.bas`, `breakout2.bas`, `minehunter.bas`

In parallel, the nuBasicEditor — a Windows IDE with syntax highlighting, autocomplete,
context help (F1), bookmarks, and a toolbar — grew from its first release (v0.2) to v1.11.

## Type System Expansion (Nov 2014 – Nov 2015)

- **Boolean** (`#` postfix) and **Long64** (`&` postfix) numeric types (v1.24)
- **Unary operators** — C-style prefix `++` and `--` (v1.24)
- **Escape sequences** — `\n`, `\r`, `\t`, `\a`, and others inside string literals (v1.24)
- **`Const` statement** — named, immutable constants with optional explicit type (v1.22)
- **`Elif` / `ElseIf`** — cleaner multi-branch conditionals (v1.25–v1.26)
- **`Stop` statement** — break execution and return to the REPL, resumable with `Cont` (v1.51)
- **Explicit type declarations** — `Dim x As Integer`, `Dim s As String` (v1.40)
- **`Struct`** — user-defined composite types with named fields and full nesting (v1.40)
- **Extended function syntax** — typed return values, functions returning struct objects and arrays (v1.42, v1.48)

New graphical examples: `clock.bas` (analog clock), `plane.bas` (flying-plane animation).

## Hash Tables, Floating-Point Formatting (Jan 2017)

Version 1.47 added:

- **Hash tables** — `HSet`, `HGet`, `HCnt`, `HChk`, `HDel`
- **`Any` type** — automatically takes the type of whatever is assigned to it
- **`StrP` / `StrP$`** — floating-point formatting with a specified number of significant digits
- **`GetAppPath()`** — path of the nuBASIC executable
- **`Quit()`** — exit the interpreter from within a program

## Linux IDE, MIT License (Jul 2017)

Version 1.48 was a strategic milestone:

- The IDE was **ported to Linux/GTK+**, making the full development environment available on both platforms
- License changed to **MIT**, removing all previous usage restrictions
- Added editor zoom in/out, `SetTopmost`, updated stb_image to 2.15, Scintilla to 3.74

## READ / DATA / RESTORE (Jul 2019)

Version 1.50 added the classic `Read`, `Data`, and `Restore` statements for embedding structured
constant tables — palettes, maps, sprite data — directly in source code.

## Quality-of-Life Fixes (Sep 2019)

Version 1.51:

- Fixed a parser regression with the unary minus operator
- Added `Stop` for compatibility with other BASIC interpreters
- Extended `If` to support an implicit `GoTo` after `Then`/`Else` without the `GoTo` keyword
- Fixed mouse position misalignment on Windows high-DPI / scaled display configurations

## Unicode Support (Feb 2022)

Version 1.52 added support for **Unicode escape sequences** in string literals: the `$u` prefix
enables `\u0000`–`\uFFFF` sequences, outputting the corresponding Unicode characters.

## Custom Windows Console, UTF-8, MSI Installer (March 2026)

Version 1.60 was the largest infrastructure release since the original:

- A **brand-new GDI-based console window** replaced the standard Windows console. Renders
  text and graphics natively using GDI, works correctly both standalone and embedded in the
  IDE, and avoids the limitations of the Windows Console API.
- **Full UTF-8 support** end-to-end: characters outside ASCII are now preserved through the
  tokenizer, interpreter runtime, and GDI console. Two silent-stripping bugs were fixed. The
  new example `hello_world_languages.bas` demonstrates multilingual output in 20 languages.
- An **MSI installer** introduced proper Windows Add/Remove Programs integration, desktop
  shortcuts, and clean uninstallation.
- Scintilla updated to its latest version.

## Breaking Changes: Removed Deprecated APIs, Parser Refactor (April 2026, v2.0)

Version 2.0 removes all deprecated built-in functions announced in v1.62 and refactors the
statement parser internals.

**Removed deprecated scalar date/time functions** — `SysDay`, `SysMonth`, `SysYear`, `SysWDay`,
`SysYDay`, `SysHour`, `SysMin`, `SysSec` are gone. Use `GetDateTime()` which returns a `DateTime`
struct with all fields in a single call.

**Removed deprecated scalar mouse functions** — `GetMouseX`, `GetMouseY`, `GetMouseBtn` are gone.
Use `GetMouse()` which returns a `Mouse` struct with fields `x`, `y`, and `btn`.

**Statement parser refactored** — the monolithic `stmt_parser_t` class has been renamed to
`statement_parser_t` and its source split into three focused files: `nu_parser_io.cc` (I/O
statements), `nu_parser_flow.cc` (control-flow statements), and `nu_parser_struct.cc`
(struct/class statements). The public header is now `include/parser/nu_statement_parser.h`.

**Bug fixes**:
- `Print #n` with a space between `Print` and `#n` now works correctly.
- `On expr GoTo lbl1, lbl2, ...` now dispatches to all labels in the list, not only the first.
- `lbl: statement` inline syntax after an `On...GoTo` dispatch no longer raises a syntax error.

---

## Struct-returning Functions, Screen Mode, Regression Tests (April 2026, v1.62)

Version 1.62 introduced new built-in functions that return all their values in a single struct
rather than requiring multiple calls, a headless text mode for scripting and CI, and the first
automated regression test suite.

**`GetDateTime()` and `GetMouse()`** replace the individual scalar accessors with struct-typed
return values. `GetDateTime()` returns a `DateTime` struct with fields `year`, `month`, `day`,
`hour`, `minute`, `second`, `wday`, and `yday`. `GetMouse()` returns a `Mouse` struct with fields
`x`, `y`, and `btn`. Both struct types are pre-registered at interpreter startup so that
`Dim dt As DateTime` and `Dim m As Mouse` work without a user-written `Struct` block. The
previous individual functions (`SysYear`, `SysMonth`, `SysDay`, `SysHour`, `SysMin`, `SysSec`,
`SysWDay`, `SysYDay`, `GetMouseX`, `GetMouseY`, `GetMouseBtn`) were deprecated in v1.62 and
removed in v2.0.

**`SCREEN` statement** — `Screen 0` switches to text/headless mode: all I/O goes through the
real Windows console (stdout/stdin) and every GDI drawing call is a silent no-op. `Screen 1`
restores full GDI console mode. Mirrors the GW-BASIC `SCREEN` command.

**`-t` / `--text-mode` CLI flag** activates `Screen 0` before the interpreter starts and
reconnects the CRT stdio streams to the caller's inherited Win32 handles (pipe, redirect,
terminal), allowing test runners and CI pipelines to capture `Print` output without opening a
GUI window. The interpreter exits cleanly after executing a batch command instead of blocking
on stdin.

**Regression test suite** — `tests/run_tests.ps1` (PowerShell) and `tests/run_tests.sh` (Bash)
execute every `test_*.bas` file in the `tests/` directory with `-t -e`, parse `PASS`/`FAIL`
lines and the summary printed by each test, and report per-suite and overall totals. A
`RunTests` CMake/Visual Studio target invokes the appropriate script from the IDE or from
`cmake --build`. Ten test suites are included: math, strings, arrays, control flow, procedures,
types, struct (ByRef), struct (file), struct arrays, and file I/O.

---

## Flicker-free Graphics Rendering (April 2026, v1.61)

Version 1.61 addressed per-primitive screen flicker in animated programs. Before this release,
every graphics call triggered an immediate blit — a frame composed of ten drawing commands
produced ten visible intermediate states.

Two fixes were implemented:

1. **Painting compositing fix** — text characters and the cursor are now composited onto the
   back buffer before the single copy to the screen.

2. **`ScreenLock` / `ScreenUnlock` / `Refresh` instructions** — a new atomic lock flag
   suppresses the automatic blit after each drawing command. Fourteen graphical example
   programs were updated: `breakout2.bas`, `breakout3.bas`, `pingpong.bas`, `nutetris.bas`,
   `nutetris3d.bas`, `raycast3d.bas`, `plane.bas`, `clock.bas`, `cartoon.bas`,
   `minesHunter.bas`, `tictactoe.bas`, `line.bas`, `rosetta.bas`, and `calc.bas`.

---

← [IDE](IDE) | Next: [Interpreter Internals](Interpreter-Internals)
