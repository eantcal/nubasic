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

## nuBASIC 2.0 — New Language Features (2026)

Version 2.0 is a major release that both removes deprecated APIs and introduces significant
new language capabilities.

### SELECT CASE

A `Select Case` / `End Select` block dispatches on any scalar expression. Each `Case` arm
is tested in order; the first matching arm executes and control passes to `End Select`.
`Case Else` is the optional fallback.

Supported arm forms: single values (`Case 1`), comma-separated lists (`Case 1, 3, 5`),
inclusive ranges (`Case 1 To 10`), comparison guards (`Case Is > 100`), and string values.
Nesting is supported.

### Classes with Static Methods

The `Class`/`End Class` block gained `Static Function` and `Static Sub` members — class-level
procedures that can be invoked without an instance:

```basic
Class Utils
   Static Function Max(a As Integer, b As Integer) As Integer
      If a > b Then Max = a Else Max = b
   End Function
End Class

Print Utils.Max(3, 7)   ' 7
```

Instance methods continue to use `Me` to refer to the receiver.

### main() Entry Point

When a `Function main(...)` is present in the top-level program, execution starts there
rather than at the first statement. Three signatures are accepted:

- `Function main() As Integer`
- `Function main(argc As Integer) As Integer`
- `Function main(argc As Integer, argv() As String) As Integer`

`argv(0)` is the script filename; remaining elements are extra arguments passed on the
command line after the filename. The entry-point behaviour is suppressed in `Include`d files
so libraries can define their own `main` helpers without hijacking execution.

### Open-ended Array Parameters

Subs and Functions can now declare a parameter with empty parentheses — `param() As Type`
— to accept an array of any caller-determined size. Previously the dimension had to be
specified in the signature, preventing generic array-processing procedures.

### Namespaced Modules

`Syntax Modern` activates a two-namespace view of the built-in library: functions are
addressable as `module::name` (e.g. `math::sin`, `string::left$`, `runtime::sizeof`).
`Using ModuleName` imports a module's names into the local scope for unqualified access.
`Syntax Legacy` (the default) restores the classic flat namespace so all existing programs
continue to work unchanged.

### Breaking Changes: Removed Deprecated APIs, Parser Refactor (April 2026, v2.0)

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
- CLI: arguments following `-e filename` are now passed to the script as `argv` entries and
  no longer corrupted into the filename string.

### Debugger, Native Calls, and Runtime Optimisation (May 2026, v2.0)

The 2.0 cycle also modernised the execution and tooling layers:

- **Debugger stepping model** — the interpreter core now exposes reusable Continue, Step Into,
  Step Over, Step Out, Pause/Break, and Run to Cursor commands. The same model is used by the
  native IDE, the debug CLI, and the VS Code debug adapter.
- **Graphics-aware VS Code debugging** — graphical programs can be debugged with drawing routed
  to a separate GDI window while debugger protocol messages and textual diagnostics remain in
  the VS Code session.
- **Native library calls** — `Declare Function ... Lib ...` can call exported functions from
  Windows DLLs, Linux shared objects, and macOS dylibs through libffi. Native calls are enabled
  by default for trusted local hosts and can be disabled with `--disable-native-calls`.
- **Line continuation** — `_` joins the following physical source line into the same logical
  BASIC statement before parsing.
- **Runtime value optimisation** — `variant_t` now stores scalar values inline, boxes
  struct/object metadata, uses copy-on-write for struct payload copies, and preserves struct
  field declaration order for native interop and predictable introspection.
- **Hex literal compatibility** — the original BASIC `&hFF` / `&HFF` syntax remains supported,
  and `0xFF` / `0XFF` is accepted as a convenience for C-style constants.

### WinRayCast Integration (June 2026, v2.0)

The headline addition for Windows is the integrated **WinRayCast** engine — a lightweight
raycaster that lets a BASIC program render pseudo-3D, *Wolfenstein 3D*-style first-person
scenes from a 2D grid map. The engine handles textured walls, sprites, doors, actors,
weapons, pickups, sound, and multi-level transitions, while nuBASIC drives it through the
`raycast` module (the `Ray…` built-in functions): `RayInit`, `RayLoadProject`,
`RayUpdate`, `RayRender`, `RayPresent`, and the player/combat/map query calls. It is
enabled by default on Windows builds (`NUBASIC_WITH_RAYCAST`) and absent on other
platforms.

This is a true engine binding, distinct from the earlier pure-BASIC software raycaster
example (`raycast3d.bas`), which kept drawing columns by hand with `Line`. The companion
demo `examples/raycast/eclipse_protocol.bas` is a complete, playable game — multi-weapon
combat, energy/health, a minimap and HUD, key/lock doors, autosave checkpoints, and
elevator transitions between levels — and showcases `Syntax Modern`, `Struct`, and `Class`
working together. See [Raycast Game Engine](Raycast-Game-Engine) for the full reference.

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
