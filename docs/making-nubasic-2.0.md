# Making nuBASIC 2.0: Runtime and Debugger Internals

This note is for developers interested in how nuBASIC 2.0 improved the
interpreter core, the debugger model, and the runtime value representation.
It focuses on why the changes matter for language quality, maintainability,
and performance.

## Design Goals

nuBASIC 2.0 keeps the original spirit of the project: a small BASIC-family
language that can still run classic numbered programs, while also supporting
structured programming, classes, graphics, projects, native calls, an integrated
pseudo-3D raycasting engine (WinRayCast, on Windows), and modern debugger
workflows.

The recent internal work had four main goals:

- Make debugger execution semantics explicit and reusable.
- Keep graphical program output separate from debugger protocol and text I/O.
- Reduce allocation and copy overhead in the central `variant_t` value type.
- Make language features easier to document, test, and reuse across the CLI,
  IDE, VS Code extension, and future tooling.

## Debugger Execution Model

Older nuBASIC debugging was mostly statement stepping: execute one BASIC
statement, stop, repeat. That is useful, but it does not model what developers
expect from modern debuggers.

The interpreter core now exposes debugger execution commands as explicit
semantics:

- Continue
- Step Into
- Step Over
- Step Out
- Pause / Break
- Run to Cursor

The important design choice is that these are interpreter concepts, not UI
concepts. The Scintilla IDE, the debug CLI, and the VS Code debug adapter can
all ask for the same command and receive consistent stop events.

This improves quality because stepping behavior is defined once, close to the
runtime state it depends on: current statement, procedure/function nesting,
GOSUB/RETURN flow, loops, call stack depth, and breakpoints. UI layers no
longer need to guess how to emulate Step Over or Step Out.

## Graphics-Aware Debugging

Graphical programs need a drawing surface, but debugger protocol messages and
text diagnostics must not be mixed with that surface. VS Code in particular
needs stdout for the debug adapter protocol and terminal output.

The Windows debug backend therefore supports a hybrid graphics mode:

- `nubasicdebug.exe` remains a console-subsystem process.
- `--graphics-window` opens a separate GDI window for graphical output.
- Debugger machine-interface events and textual diagnostics stay on stdout.
- Breakpoints, stepping, pause, and program-counter reporting remain part of
  the normal debugger flow.

This separation improves both user experience and tooling reliability. A game
or animation can be debugged without stealing the communication channel used by
the IDE or VS Code adapter.

## Runtime Value Optimisation

`variant_t` is the universal value type of the interpreter. Every expression
evaluates to a `variant_t`, and every variable stores one. That makes it a
critical performance path: arithmetic, comparisons, strings, arrays, structs,
objects, procedure calls, and built-in functions all move through this type.

The previous representation used a vector-backed payload even for scalar
values. A simple integer or double often paid the cost of a one-element vector
allocation. That was simple, but expensive in hot paths such as expression
evaluation and compound assignment.

The new representation keeps the same public behavior while improving the
internal layout:

- Scalar integers, doubles, booleans, and strings use inline storage.
- Array values still use the vector payload path because they need indexed
  storage.
- Struct and object metadata is boxed, so ordinary scalar values do not carry
  that memory footprint.
- Struct payload copies use copy-on-write, so pass-by-value and assignment are
  cheap until a copied value is actually mutated.
- Struct field storage preserves declaration order, which is important for
  native interop and predictable introspection.

The language benefit is that richer value semantics become cheaper. Classes,
structs, destructors, native calls, and debugger watches can all rely on the
same value type without forcing scalar-heavy code to pay for the heaviest
cases.

## Compound Assignment

Compound assignments such as `+=`, `-=`, `*=`, and `/=` now have direct
mutation paths for common scalar cases. Instead of always computing a temporary
result and assigning it back, integer and double values can be updated in
place when no type promotion is required.

The fallback path still uses the normal operators when promotion is necessary,
for example when `Integer += Double` must produce a double result. This keeps
the semantics predictable while making the common case faster.

## Native Calls and Line Continuation

Native library calls add a bridge from BASIC code to platform APIs:

```basic
Declare Function GetCurrentProcessId Lib "kernel32.dll" () As DWORD
```

The feature uses libffi for invocation and platform loaders such as
`LoadLibraryW` on Windows and `dlopen` on POSIX. It is enabled by default for
trusted local hosts and can be disabled with `--disable-native-calls`.

Line continuation with `_` was added partly to make native declarations and
long calls readable:

```basic
Declare Function MessageBoxA Lib "user32.dll" _
    Alias "MessageBoxA" _
    (hwnd As Pointer, text As String, caption As String, flags As DWORD) As Integer
```

The parser joins continued physical lines before parsing the logical BASIC
statement. Debuggers stop on the first physical line of the joined statement,
which keeps source navigation deterministic.

## Hex Literal Compatibility

nuBASIC preserves the original BASIC hexadecimal syntax:

```basic
color% = &hFFFFFF
```

It also accepts C-style hexadecimal literals:

```basic
color% = 0xFFFFFF
```

This does not replace the BASIC form. It makes imported constants from C
headers, Win32 documentation, POSIX examples, and graphics code easier to paste
into nuBASIC programs.

## WinRayCast Integration

The most visible addition of the 2.0 cycle for Windows is the integrated
**WinRayCast** engine: a lightweight raycaster that renders pseudo-3D,
*Wolfenstein 3D*-style first-person scenes from a 2D grid map. The design
deliberately keeps the engine and the language layers separate.

- The C++ engine (under `raycast/`) owns rendering, the world map, sprites,
  actors, doors, weapons, pickups, sound, and multi-level transitions. It is
  built into a static library and linked into the interpreter only when
  `NUBASIC_WITH_RAYCAST` is enabled (the default on Windows, off elsewhere).
- The BASIC binding lives in `lib/api/nu_builtin_module_raycast.cc` as the
  `raycast` built-in module. It exposes the `Ray…` functions and holds a single
  per-process `raycast_session_t` — the world, engine, actors, weapon inventory,
  energy, collected-item counters, and pending layer transition. BASIC code never
  sees the C++ objects directly; it drives them through small, query-and-command
  style functions.
- Rendering is two-phase: `RayRender` fills an off-screen framebuffer, and
  `RayPresent` blits a rectangle of that framebuffer into the GDI window via
  `StretchDIBits`. This keeps the engine platform-neutral (it just produces a
  pixel buffer) while the Windows-specific presentation stays in the binding,
  guarded by `#ifdef _WIN32`. `RayKeyDown` similarly wraps `GetAsyncKeyState`.
- Worlds and multi-level projects are described in JSON (`*.world.json`) and
  loaded relative to a base directory, so scene data is data, not code.

When the engine is compiled out, only `RayAvailable()` is registered — returning
`0` — so portable programs can probe for support and degrade gracefully. The
companion demo `examples/raycast/eclipse_protocol.bas` is written as a tutorial
and exercises most of the API while combining `Syntax Modern`, `Struct`, and
`Class`. The full `Ray…` reference is on the project wiki
(*Raycast Game Engine*).

## Testability

The internal work is backed by regression tests at several levels:

- General language tests run through the CLI in text mode.
- Debugger contract tests exercise the machine-interface backend.
- Graphics debugger contract tests verify that graphical debug sessions still
  report breakpoints and step events correctly.
- Interruptible input tests cover interactive `INPUT` and `INPUT$` behavior.
- Runtime tests cover value semantics such as structs, classes, destructors,
  object references, and numeric/string behavior.

This matters because the optimized runtime representation is deliberately
invisible to BASIC programs. Tests protect that contract: performance can
improve, but observable language behavior must stay stable.

## Why This Improves the Language

These changes are not only optimisations. They make nuBASIC more coherent as a
language implementation:

- Debugger behavior is part of the interpreter model, not a UI workaround.
- Graphical programs can be debugged using the same execution semantics as
  text programs.
- Scalar-heavy BASIC code avoids unnecessary heap traffic.
- Struct and object support becomes more practical for larger programs.
- Native API examples can be written in readable BASIC instead of dense buffer
  manipulation.
- The same interpreter core can serve the CLI, native IDE, VS Code extension,
  tests, and future tools.

The result is a runtime that remains small and understandable, but is now much
better prepared for modern workflows and larger BASIC programs.
