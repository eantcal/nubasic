# Introduction to nuBASIC

← [BASIC: A Language Built for Learning](BASIC-A-Language-Built-for-Learning) | Next: [Getting Started](Getting-Started)

---

nuBASIC is a modern, open-source BASIC interpreter written in C++20, created by Antonino
Calderone and first released in 2014. It is firmly rooted in the BASIC tradition — interactive,
welcoming, and immediately productive — while adding the structured programming features and
graphical capabilities that contemporary programs require.

## nuBASIC as a BASIC Dialect

nuBASIC belongs to the family of structured BASIC dialects descended from QuickBASIC and its
successors. Like them, it accepts both old-style programs with line numbers and new-style programs
that rely entirely on labels, `Sub`, `Function`, and modern control structures. The two styles can
even be mixed in the same file.

What nuBASIC adds on top of the classic BASIC foundation:

- **Rich type system** — Integer (`%`), Double (no suffix), Boolean (`#`), Long64 (`&`),
  String (`$`), Byte (`@`), `Any` (type-deduced), and user-defined `Struct` types
- **Structured control flow** — `If/ElIf/Else/End If`, `While/Wend`, `Do/Loop While`,
  `Exit For/While/Do/Sub/Function`
- **First-class subroutines and functions** — with typed parameters, typed return values,
  and recursion
- **Arrays and hash tables** built into the language
- **Full file I/O** — sequential, binary, and random-access
- **Graphics** — lines, rectangles, ellipses, filled shapes, pixel access, bitmaps, text
- **Double-buffered rendering** — `ScreenLock`/`ScreenUnlock`/`Refresh` for flicker-free animation
- **Screen mode switching** — `Screen 0` (text/headless, stdout/stdin) / `Screen 1` (GDI); `-t` flag for CI pipelines
- **Mouse input** — `GetMouse()` returns a `Mouse` struct with `x`, `y`, `btn` in one call
- **Date/time** — `GetDateTime()` returns a `DateTime` struct with all fields in one call
- **UTF-8** string literals and console output, including escape sequences for Unicode characters
- **Built-in interactive help** — `Help keyword` and `Apropos topic` from the REPL

## Platforms

The Windows build provides a dedicated GDI-based console window that renders text and graphics
natively, works both standalone and embedded inside the nuBASIC IDE, and supports all graphical
APIs including pixel-level drawing. The Linux build uses the terminal for text and X11 for
graphics. A *console* build variant (without graphics, sound, or window manager support) is also
available for constrained environments.

## Console Mode and the Console Build

### Console Mode

When nuBASIC starts without a file argument it enters **console mode** — the interactive
Read-Eval-Print Loop (REPL). This is the primary working environment for exploring the
language, writing and testing short programs, and debugging longer ones.

In console mode, nuBASIC accepts two kinds of input:

- **Immediate statements** — typed without a line number, executed the moment you press Enter:

  ```
  nubasic> Print Sin(PI() / 6)
   0.5
  nubasic> x% = 42 : Print x% * x%
   1764
  nubasic> Help Mid$
  ```

- **Numbered lines** — lines prefixed with an integer are stored in the program buffer:

  ```
  nubasic> 10 For i%=1 To 5
  nubasic> 20   Print i% * i%
  nubasic> 30 Next i%
  nubasic> Run
   1
   4
   9
   16
   25
  ```

The console mode command set includes everything needed for a complete edit-run-debug cycle:

| Category | Commands |
|----------|----------|
| Execution | `Run`, `Cont`, `Resume`, `End` |
| Editing | `List`, `New`, `Clr`, `Renum`, `Grep` |
| Files | `Load`, `Save`, `Exec`, `Pwd`, `Cd` |
| Debugging | `Break`, `RmBrk`, `ClrBrk`, `TrOn`, `TrOff`, `StOn`, `StOff`, `Vars`, `Stop` |
| Help | `Help`, `Apropos`, `Ver`, `Meta` |
| System | `!` (shell), `Exit` |

### The Console Build

The **console build** is a stripped-down variant of nuBASIC compiled without graphics, sound,
mouse, or window manager support. All graphical instructions and their related functions are
absent. What remains is a fully functional text-mode interpreter: the complete language core,
all string and math functions, file I/O, hash tables, `Sub`/`Function`, `Struct`, and the full
console mode with all debugging commands.

The console build is the right choice when:

- Running on a **headless server** or an embedded system with no display
- **Scripting** text-processing or file-manipulation tasks
- **Teaching** pure programming concepts without graphical distractions
- **Constrained environments** where a smaller binary footprint matters

## Why nuBASIC?

**It is genuinely simple.** A complete beginner with no prior programming experience can be
writing and running programs within minutes. There is no project system to configure, no
compiler to invoke, no runtime to install separately. The interactive REPL gives immediate
feedback. Error messages are plain English. The built-in help system is always one command away.

**It is also a complete language.** Structures, typed parameters, recursion, hash tables, file
I/O, graphics, and double-buffered animation are all available when you need them. The same
interpreter that runs a beginner's first `Print "Hello"` also runs a 3D ray-caster, a Tetris
clone, and a Mandelbrot set renderer.

The same algorithm, two styles — both valid nuBASIC:

**Classic style:**
```basic
5   Rem rosetta_classic.bas
10  For x0 = -2 To 2 Step .013
20  For y0 = -1.5 To 1.5 Step .013
30  x = 0 : y = 0 : iteration = 0 : maxIteration = 223
70  xtemp = x*x - y*y + x0
80  y = 2*x*y + y0
90  x = xtemp : iteration = iteration + 1
110 If ((x*x + y*y <= 4) And (iteration < maxIteration)) Then GoTo 70
120 If iteration <> maxIteration Then c = iteration Else c = 0
130 d% = 150 : dx% = 300 : dy% = 300
140 FillRect x0*d%+dx%, y0*d%+dy%, x0*d%+dx%+2, y0*d%+dy%+2, Int(c)*16
150 Next y0
160 Next x0
```

**Structured style:**
```basic
' rosetta_structured.bas
For x0 = -2 To 2 Step .013
   ScreenLock
   For y0 = -1.5 To 1.5 Step .013
      x = 0 : y = 0 : iteration = 0 : maxIteration = 223

      While ((x*x + y*y <= 4) And (iteration < maxIteration))
         xtemp = x*x - y*y + x0
         y = 2*x*y + y0
         x = xtemp
         ++iteration
      End While

      If iteration <> maxIteration Then c = iteration Else c = 0
      FillRect x0*150+300, y0*150+300, x0*150+302, y0*150+302, Int(c)*16
   Next y0
   ScreenUnlock
Next x0
```

## Key Features at a Glance

- Structured programming: `Sub`, `Function`, `For`, `While`, `Do…Loop While`, `If/ElIf/Else`
- Rich type system: Integer, Double, Boolean, Long64, String, Any, user-defined `Struct`
- Arrays and hash tables
- Full file I/O (sequential, binary, and random access)
- Graphics: lines, rectangles, ellipses, text, bitmaps, pixel access
- Flicker-free double-buffered rendering (`ScreenLock` / `ScreenUnlock` / `Refresh`)
- Screen mode switching: `Screen 0` (headless/text) / `Screen 1` (GDI); `-t` CLI flag for CI
- Mouse and keyboard input via `GetMouse()` struct function
- Date/time via `GetDateTime()` struct function
- UTF-8 string literals and console output
- Built-in help (`Help`, `Apropos`) accessible directly from the REPL
- Automated regression test suite (`tests/run_tests.ps1` / `run_tests.sh`)

---

← [BASIC: A Language Built for Learning](BASIC-A-Language-Built-for-Learning) | Next: [Getting Started](Getting-Started)
