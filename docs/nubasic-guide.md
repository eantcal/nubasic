# nuBASIC — User Guide

> Version 1.62 · http://www.nubasic.eu/
> Author: Antonino Calderone — antonino.calderone@gmail.com

---

## Table of Contents

1. [BASIC: A Language Built for Learning](#1-basic-a-language-built-for-learning)
2. [Introduction to nuBASIC](#2-introduction-to-nubasic)
3. [Getting Started](#3-getting-started)
4. [The nuBASIC Language](#4-the-nubasic-language)
   - 4.1 [Variables and Types](#41-variables-and-types)
   - 4.2 [Operators](#42-operators)
   - 4.3 [Control Flow](#43-control-flow)
   - 4.4 [Subroutines and Functions](#44-subroutines-and-functions)
   - 4.5 [Classes and Objects](#45-classes-and-objects)
   - 4.6 [main() Entry Point](#46-main-entry-point)
   - 4.7 [Namespaced Modules](#47-namespaced-modules)
   - 4.8 [Structures](#48-structures)
   - 4.9 [Arrays](#49-arrays)
   - 4.10 [Hash Tables](#410-hash-tables)
   - 4.11 [File I/O](#411-file-io)
   - 4.12 [DATA, READ, RESTORE](#412-data-read-restore)
   - 4.13 [String Handling](#413-string-handling)
5. [Graphics and Multimedia](#5-graphics-and-multimedia)
   - 5.1 [Drawing Primitives](#51-drawing-primitives)
   - 5.2 [Flicker-free Rendering — ScreenLock / ScreenUnlock / Refresh](#52-flicker-free-rendering)
   - 5.3 [Mouse Input](#53-mouse-input)
   - 5.4 [Sound and Window Management](#54-sound-and-window-management)
6. [Command Reference](#6-command-reference)
   - 6.1 [Console Commands (REPL / CLI)](#61-console-commands)
   - 6.2 [Instructions](#62-instructions)
   - 6.3 [Built-in Functions](#63-built-in-functions)
   - 6.4 [Operators Reference](#64-operators-reference)
7. [Integrated Development Environment (IDE)](#7-integrated-development-environment-ide)
   - 7.1 [Syntax Highlighting Editor](#71-syntax-highlighting-editor)
   - 7.2 [Keyboard Commands](#72-keyboard-commands)
   - 7.3 [Code Folding](#73-code-folding)
   - 7.4 [Find and Replace](#74-find-and-replace)
   - 7.5 [Bookmarks](#75-bookmarks)
   - 7.6 [Auto-Completion](#76-auto-completion)
   - 7.7 [Toolbar](#77-toolbar)
   - 7.8 [Context-Sensitive Help and Online Help](#78-context-sensitive-help-and-online-help)
   - 7.9 [Integrated Debugger](#79-integrated-debugger)
8. [History and Evolution of nuBASIC](#8-history-and-evolution-of-nubasic)
9. [Making nuBASIC: Interpreter Internals](#9-making-nubasic-interpreter-internals)
   - 9.1 [Main Components](#91-main-components)
   - 9.2 [A Line-Oriented Interpreter](#92-a-line-oriented-interpreter)
   - 9.3 [Tokens and the Tokenizer](#93-tokens-and-the-tokenizer)
   - 9.4 [Token List Container](#94-token-list-container)
   - 9.5 [Parsing the Code](#95-parsing-the-code)
   - 9.6 [Expression Parsing in Detail](#96-expression-parsing-in-detail)
   - 9.7 [The Variant Type](#97-the-variant-type)
   - 9.8 [Tracing Execution of a Simple Program](#98-tracing-execution-of-a-simple-program)
   - 9.9 [Extending the Built-in Function Set](#99-extending-the-built-in-function-set)
10. [Building nuBASIC from Source](#10-building-nubasic-from-source)
    - 10.1 [Getting the Source](#101-getting-the-source)
    - 10.2 [Build Targets Overview](#102-build-targets-overview)
    - 10.3 [Building on Windows](#103-building-on-windows)
    - 10.4 [Building on Linux](#104-building-on-linux)
    - 10.5 [Building on macOS](#105-building-on-macos)
    - 10.6 [Building on iOS with iSH](#106-building-on-ios-with-ish)
    - 10.7 [CMake Option Reference](#107-cmake-option-reference)
    - 10.8 [Creating Installers and Packages](#108-creating-installers-and-packages)

---

## 1. BASIC: A Language Built for Learning

### Origins

BASIC — **B**eginner's **A**ll-purpose **S**ymbolic **I**nstruction **C**ode — was designed in 1964
at Dartmouth College by John G. Kemeny and Thomas E. Kurtz. Their goal was simple and radical for
its time: give students with no prior programming experience a language they could learn and use
productively in a single sitting. Every design decision was made with that beginner in mind.

Unlike Fortran or COBOL, which required separate compilation steps and knowledge of job-control
languages, BASIC ran interactively. You typed a line, you pressed Enter, you got a result. You
numbered your lines, typed `RUN`, and the program executed immediately. There was no barrier
between thought and execution.

### The era of home computing

BASIC became the dominant programming language of the personal computer revolution in the late
1970s and early 1980s. Almost every home computer — the Apple II, the Commodore 64, the Sinclair
Spectrum, the TRS-80, the BBC Micro — shipped with a BASIC interpreter built into ROM. For an
entire generation, BASIC was not merely a programming language; it was *the* way to interact with
a computer beyond running packaged software.

Programs were short, the screen was immediately visible, and the feedback loop was instant. A
ten-year-old could write a game. A scientist could write a simulation. BASIC democratized
programming long before the web existed.

The classical dialect of that era had a distinctive style:

```basic
10 PRINT "ENTER YOUR NAME:"
20 INPUT N$
30 PRINT "HELLO, "; N$; "!"
40 GOTO 10
```

Every line had a number, flow was controlled with `GOTO` and `GOSUB`, and variables were short.
The line numbers served double duty: they were the addresses used by `GOTO`, and they defined the
order in which the program was stored and listed.

### Dialects and evolution

BASIC was never a single, standardised language. Each hardware vendor wrote their own interpreter,
and dialects multiplied. Microsoft's BASIC (later GW-BASIC and QuickBASIC) became the most
widespread on IBM-compatible PCs. Turbo Basic, PowerBASIC, and eventually Visual Basic followed,
each adding structured programming features, better type systems, and graphical capabilities.

The structured revolution in BASIC arrived in the mid-1980s: `WHILE`/`WEND` replaced `GOTO`-heavy
loops, `SUB` and `FUNCTION` replaced line-numbered subroutines, and compilers made BASIC programs
run at near-native speed. Modern successors such as QB64, FreeBASIC, and PureBasic carry that
tradition forward today.

### What all BASIC dialects share

Despite the diversity, most BASIC interpreters share a recognisable core:

- Case-insensitive keywords (`PRINT`, `Print`, `print` are all the same)
- Variables need little or no declaration; a suffix character often signals the type
  (`$` for strings, `%` for integers)
- The `PRINT` statement sends output to the screen
- `INPUT` reads from the keyboard
- `IF … THEN … ELSE` for decisions
- `FOR … TO … NEXT` for counted loops
- `GOTO` and `GOSUB`/`RETURN` for flow control
- Line numbers are accepted but not always required

---

## 2. Introduction to nuBASIC

nuBASIC is a modern, open-source BASIC interpreter written in C++11, created by Antonino
Calderone and first released in 2014. It is firmly rooted in the BASIC tradition — interactive,
welcoming, and immediately productive — while adding the structured programming features and
graphical capabilities that contemporary programs require.

### nuBASIC as a BASIC dialect

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
- **Mouse input** — position and button state readable from BASIC code
- **UTF-8** string literals and console output, including escape sequences for Unicode characters
- **Built-in interactive help** — `Help keyword` and `Apropos topic` from the REPL

### Platforms

The Windows build provides a dedicated GDI-based console window that renders text and graphics
natively, works both standalone and embedded inside the nuBASIC IDE, and supports all graphical
APIs including pixel-level drawing. The Linux build uses the terminal for text and X11 for
graphics. A *console* build variant (without graphics, sound, or window manager support) is also
available for constrained environments.

### The Console Mode and the Console Build

#### Console mode

When nuBASIC starts without a file argument it enters **console mode** — the interactive
Read-Eval-Print Loop (REPL). This is the primary working environment for exploring the
language, writing and testing short programs, and debugging longer ones.

In console mode, nuBASIC accepts two kinds of input:

- **Immediate statements** — typed without a line number, executed the moment you press Enter.
  Results appear immediately. This is the fastest way to experiment with a function or test
  a calculation:

  ```
  nubasic> Print Sin(PI() / 6)
   0.5
  nubasic> x% = 42 : Print x% * x%
   1764
  nubasic> Help Mid$
  ```

- **Numbered lines** — lines prefixed with an integer are stored in the program buffer instead
  of being executed. Once you have entered enough lines, type `Run` to execute the program,
  `List` to review it, `Save` to write it to a file, or `New` to clear it and start over:

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

The two modes coexist naturally: you can load a file with `Load`, inspect it with `List`,
set a breakpoint with `Break`, run it with `Run`, and then examine variable values with `Vars`
— all without leaving the interpreter.

The console mode command set includes everything needed for a complete edit-run-debug cycle:

| Category | Commands |
|----------|----------|
| Execution | `Run`, `Cont`, `Resume`, `End` |
| Editing | `List`, `New`, `Clr`, `Renum`, `Grep` |
| Files | `Load`, `Save`, `Exec`, `Pwd`, `Cd` |
| Debugging | `Break`, `RmBrk`, `ClrBrk`, `TrOn`, `TrOff`, `StOn`, `StOff`, `Vars`, `Stop` |
| Help | `Help`, `Apropos`, `Ver`, `Meta` |
| System | `!` (shell), `Exit` |

On Windows, the console is rendered by a dedicated GDI window that supports both text output
and pixel-level graphics in the same surface. The window can be moved and resized from within
a program using `MoveWindow`. On Linux, the terminal handles text I/O while an X11 window
provides the graphical surface when graphics instructions are used.

#### The console build

The **console build** is a stripped-down variant of nuBASIC compiled without graphics, sound,
mouse, or window manager support. All graphical instructions (`Line`, `FillRect`, `TextOut`,
`ScreenLock`, `PlotImage`, …) and their related functions (`GetMouseX`, `GetSWidth`,
`MoveWindow`, `PlaySound`, `MsgBox`, …) are absent.

What remains is a fully functional text-mode interpreter: the complete language core, all
string and math functions, file I/O, hash tables, `Sub`/`Function`, `Struct`, and the full
console mode with all debugging commands. Programs that do not use graphics run identically
on both builds.

The console build is the right choice when:

- Running on a **headless server** or an embedded system with no display
- **Scripting** text-processing or file-manipulation tasks where graphics are irrelevant
- **Teaching** pure programming concepts without the distraction of a graphical environment
- **Constrained environments** where the smaller binary footprint or reduced dependencies matter

To check at run time which build is active, use `GetPlatId()` (returns 1 on Windows, 2 on
Linux) and test for the presence of a graphics function before calling it — or simply design
programs to be graphics-free when portability across both builds is required.

### Why nuBASIC?

Two qualities make nuBASIC worth choosing over other options, and they pull in opposite directions
in a way that is usually hard to reconcile.

**It is genuinely simple.** A complete beginner with no prior programming experience can be
writing and running programs within minutes. There is no project system to configure, no
compiler to invoke, no runtime to install separately. You type a line and press Enter. The
interactive REPL gives immediate feedback. Error messages are plain English. The built-in help
system is always one command away. These are not afterthoughts — they are the core design
philosophy inherited directly from the BASIC tradition.

**It is also a complete language.** Structures, typed parameters, recursion, hash tables, file
I/O, graphics, and double-buffered animation are all available when you need them. A program
does not outgrow nuBASIC just because it becomes more ambitious. The same interpreter that runs
a beginner's first `Print "Hello"` also runs a 3D ray-caster, a Tetris clone, and a Mandelbrot
set renderer.

The third reason is flexibility of style. nuBASIC accepts both classic line-numbered programs
and modern structured code, and allows the two to coexist in the same file. This is not merely
a compatibility concession — it means that a beginner can start with the simplest possible
syntax and gradually adopt structured constructs as their understanding grows, without ever
having to relearn the language or switch tools.

The Mandelbrot set (known as the "Rosetta stone" of fractal geometry, because every language
implements it as a demonstration) illustrates this perfectly. Here is the same computation
written in both styles, both of which run correctly on nuBASIC:

**Classic style — line numbers, GoTo, inline expressions:**

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

**Structured style — labels, While, If/End If, named variables:**

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

The structured version is easier to read and maintain; the classic version is more compact
and closer to how the algorithm might appear in a 1980s textbook. nuBASIC runs both without
modification.

Internally, nuBASIC is built from two complementary components that mirror the structure of
any well-designed language:

- **The language core** — the elementary constructs: variable declarations, operators, control
  flow (`If`, `For`, `While`, `Do`), procedure definitions (`Sub`, `Function`), and type
  support (`Struct`, `Dim`, `Const`).
- **The built-in function library** — the standard capabilities that programs need without
  having to implement from scratch: math (`Sin`, `Cos`, `Sqr`, `Pow`, …), string manipulation
  (`Mid$`, `InStr`, `UCase$`, …), file I/O, system time, graphics, and mouse input.

This separation makes nuBASIC easy to extend and easy to learn in stages: master the language
core first, then explore the library as needed.

### Key features at a glance

- Structured programming: `Sub`, `Function`, `For`, `While`, `Do…Loop While`, `If/ElIf/Else`
- Rich type system: Integer, Double, Boolean, Long64, String, Any, user-defined `Struct`
- Arrays and hash tables
- Full file I/O (sequential, binary, and random access)
- Graphics: lines, rectangles, ellipses, text, bitmaps, pixel access
- Flicker-free double-buffered rendering (`ScreenLock` / `ScreenUnlock`)
- Mouse and keyboard input
- UTF-8 string literals and console output
- Built-in help (`Help`, `Apropos`) accessible directly from the REPL

---

## 3. Getting Started

### Running the interpreter

```
nubasic                   # interactive REPL
nubasic myprogram.bas     # load and run a file
nubasic -e myprogram.bas  # alternative: exec from command line
```

The interactive REPL (Read-Eval-Print Loop) accepts both single statements and complete programs.
You can type a statement and press Enter to execute it immediately, or enter numbered lines to
build up a program in memory and then type `RUN`.

### Your first program

Type directly in the REPL or save to a `.bas` file:

```basic
Print "Hello, world!"
```

With line numbers (classic BASIC style):

```basic
10 Print "Hello, world!"
20 End
```

Line numbers are entirely optional. Modern nuBASIC programs omit them and use structured control
flow instead. The two styles can coexist — nuBASIC accepts a mix of numbered and unnumbered lines
in the same source file.

Comments begin with a single quote `'` or the keyword `Rem`:

```basic
' This is a comment
Rem This is also a comment
Print "Hello"   ' inline comment
```

Multiple statements can appear on a single line separated by a colon:

```basic
x% = 5 : y% = 10 : Print x% + y%
```

### Shebang (Linux/macOS)

Make a `.bas` file directly executable on Unix-like systems by adding a shebang line as the
very first line of the file:

```basic
#!/usr/local/bin/nubasic
Print "Hello from a script!"
```

Then mark the file as executable: `chmod +x myscript.bas && ./myscript.bas`

### Built-in help

The REPL includes a complete built-in help system. You never need to leave the interpreter to
look up a keyword:

```
Help               ' list all keywords grouped by category
Help Print         ' full description and syntax of Print
Help ScreenLock    ' description of the ScreenLock instruction
Help examples      ' list installed example programs with load/run hints
Apropos mouse      ' search all help descriptions for the word "mouse"
```

---

## 4. The nuBASIC Language

### 4.1 Variables and Types

Variables in nuBASIC can be declared explicitly with `Dim` or created implicitly on first
assignment. The **type suffix** appended to the variable name determines its storage type. The
same name with different suffixes creates distinct, unrelated variables.

| Suffix | Type    | Range / Notes                         | Example            |
|--------|---------|---------------------------------------|--------------------|
| `%`    | Integer | 32-bit signed, −2 147 483 648 .. +2 147 483 647 | `count% = 10` |
| `&`    | Long64  | 64-bit signed integer                 | `big& = 10000000000` |
| `#`    | Boolean | `True` or `False`                     | `flag# = True`     |
| `$`    | String  | Variable-length UTF-8 text            | `name$ = "nuBASIC"` |
| `@`    | Byte    | 0..255; used in byte arrays           | `buf@(255)`        |
| *(none)* | Double | 64-bit IEEE 754 floating-point       | `pi = 3.14159`     |

The `Any` type is inferred automatically from the assigned value and is useful in hash tables
and generic functions.

Explicit declaration with optional type annotation — required before using arrays, and good
practice for clarity:

```basic
Dim counter As Integer
Dim name    As String
Dim ratio   As Double
Dim items(99) As Integer   ' array of 100 integers (indices 0..99)
Dim flag    As Boolean
```

Constants are declared with `Const` and cannot be reassigned after declaration. They may include
an explicit type or let the interpreter infer it:

```basic
Const MAX_SIZE As Integer = 100
Const APP_NAME$ = "nuBASIC Demo"
Const PI_APPROX = 3.14159265
```

### 4.2 Operators

#### Arithmetic operators

nuBASIC provides the standard arithmetic operators. Integer division discards the fractional part
and can be written either as `\` or as the keyword `Div`. The modulo operator `Mod` returns the
remainder. The `^` operator raises a number to a power.

```basic
a = 10 + 3     ' 13      — addition
a = 10 - 3     ' 7       — subtraction
a = 10 * 3     ' 30      — multiplication
a = 10 / 3     ' 3.333…  — floating-point division
a = 10 \ 3     ' 3       — integer division (truncates)
a = 10 Div 3   ' 3       — same as backslash
a = 10 Mod 3   ' 1       — remainder
a = 2 ^ 8      ' 256     — exponentiation
```

Increment and decrement prefixes (C-style) modify a variable in place:

```basic
++counter%     ' equivalent to: counter% = counter% + 1
--counter%     ' equivalent to: counter% = counter% - 1
```

#### Comparison operators

All comparison operators return a numeric truth value (non-zero for true, zero for false) that
can be used directly in `If`, `While`, and other conditional contexts.

```basic
If a = b  Then ...   ' equal to
If a <> b Then ...   ' not equal to
If a < b  Then ...   ' less than
If a > b  Then ...   ' greater than
If a <= b Then ...   ' less than or equal to
If a >= b Then ...   ' greater than or equal to
```

#### Logical operators

`And`, `Or`, `Xor` work on boolean expressions. `Not` inverts a boolean value. These are
short-circuit evaluated: `And` stops at the first false operand, `Or` at the first true one.

```basic
If x > 0 And y > 0  Then Print "both positive"
If x = 0 Or  y = 0  Then Print "at least one zero"
If Not(flag#)        Then Print "flag is false"
result# = (a > b) Xor (c > d)
```

#### Bitwise operators

For low-level bit manipulation, nuBASIC provides a complete set of bitwise operators. These
operate on the integer representation of their operands.

```basic
result = a bAnd b    ' bitwise AND  — bits set in both
result = a bOr  b    ' bitwise OR   — bits set in either
result = a bXor b    ' bitwise XOR  — bits set in one but not both
result = bNot(a)     ' bitwise NOT  — all bits flipped
result = a bShl 2    ' shift left 2 positions  (multiply by 4)
result = a bShr 2    ' shift right 2 positions (divide by 4)
```

Hexadecimal literals are written with the `&h` prefix and are particularly useful for colors
and bitmasks:

```basic
mask%   = &hFF000000   ' top byte only
red%    = &h0000FF     ' GDI: blue channel in low byte
green%  = &h00FF00
blue%   = &hFF0000
white%  = &hFFFFFF
black%  = &h000000
```

### 4.3 Control Flow

Control flow determines in what order the statements of a program execute. nuBASIC supports
both the classic BASIC style (line numbers, `GoTo`, `GoSub`) and fully structured programming
(`If/End If`, `For/Next`, `While/Wend`, `Sub`, `Function`). The two styles can be freely mixed.

#### If / ElIf / Else

The `If` statement is the primary decision-making construct. The single-line form is convenient
for short guards; the multi-line form (terminated by `End If`) allows multiple statements in
each branch and an unlimited chain of `ElIf` conditions.

Single-line form — both the `Then` branch and the optional `Else` branch fit on one line:

```basic
If score% > 100 Then Print "High score!" Else Print "Keep trying"
```

Multi-line form — each branch can contain any number of statements:

```basic
If score% >= 90 Then
   Print "Excellent"
   grade$ = "A"
ElIf score% >= 75 Then
   Print "Good"
   grade$ = "B"
ElIf score% >= 60 Then
   Print "Passed"
   grade$ = "C"
Else
   Print "Failed"
   grade$ = "F"
End If
```

`ElseIf` is accepted as an alias for `ElIf`. An `If` inside another `If` (nesting) is fully
supported to any depth.

#### For / Next

The `For` loop executes a block of statements a fixed number of times, advancing a counter
variable from a start value to an end value. The optional `Step` clause sets the increment;
if omitted, the increment defaults to 1. A negative `Step` counts down.

```basic
' Count from 1 to 10
For i% = 1 To 10
   Print i%
Next i%

' Custom step: floating-point counter
For x = 0.0 To 1.0 Step 0.25
   Print x          ' prints 0.0, 0.25, 0.5, 0.75, 1.0
Next x

' Count down
For i% = 10 To 1 Step -1
   Print i%
Next i%

' Exit the loop early when a condition is met
For i% = 1 To 1000
   If i% Mod 7 = 0 And i% Mod 11 = 0 Then
      Print "First multiple of both 7 and 11: "; i%
      Exit For
   End If
Next i%
```

`For` loops can be nested to any depth. The variable name after `Next` is optional but makes
nested loops easier to read.

#### While / Wend

The `While` loop repeats its body as long as the condition evaluates to true (non-zero). The
condition is tested *before* the body executes, so if it is false from the start the body never
runs. `Wend` and `End While` are interchangeable as the closing keyword.

```basic
' Read lines from a file until EOF
While Not(EOF(filenum%))
   Input# filenum%, line$
   Print line$
Wend

' Infinite loop with explicit exit condition
While 1
   key$ = InKey$()
   If key$ = "q" Or key$ = "Q" Then Exit While
   MDelay 10
Wend
```

#### Do / Loop While

The `Do` loop is the post-condition counterpart of `While`: it always executes its body at
least once, then checks the condition at the bottom. This is the natural choice when you need
to perform an action and then decide whether to repeat it.

```basic
' Wait for any key press — body runs at least once
Do
   key$ = InKey$()
Loop While Len(key$) = 0

' Retry up to 10 times
Do
   ++attempts%
   result% = TryOperation()
   If result% = 0 Then Exit Do    ' success — leave early
Loop While attempts% < 10

If result% <> 0 Then Print "Failed after 10 attempts"
```

#### GoTo / GoSub / Return

`GoTo` performs an unconditional jump to a line number or a named label. Labels are identifiers
followed by a colon and can appear anywhere in the program. While `GoTo` is often discouraged
in modern code — it makes the flow hard to follow — it remains fully supported and is sometimes
the clearest way to express a state machine or an old-style program.

```basic
GoTo gameLoop

init:
   score% = 0
   lives% = 3

gameLoop:
   ' main loop body
   If lives% = 0 Then GoTo gameOver
   GoTo gameLoop

gameOver:
   Print "Game over! Score: "; score%
   End
```

`GoSub` jumps to a subroutine identified by a line number or label, and `Return` jumps back to
the statement immediately following the `GoSub` call. Unlike `Sub` (see below), `GoSub`
subroutines share the same variable scope as the caller — there are no parameters or local
variables. `GoSub` is the classic BASIC way to share a reusable block of code without the
overhead of a full procedure definition.

```basic
GoSub DrawScreen
GoSub UpdateScore
GoTo mainLoop

DrawScreen:
   Cls
   FillRect 0, 0, 640, 480, &h000000
   Return

UpdateScore:
   TextOut 10, 10, "Score: " + Str$(score%), &hffffff
   Return
```

#### On / GoTo (computed branch)

`On expr GoTo label1, label2, …` selects a jump target based on the integer value of an
expression. If the expression evaluates to 0, control goes to `label1`; if 1, to `label2`;
and so on. This is the classic BASIC computed branch, equivalent to a switch/case without
fall-through.

```basic
' Select difficulty screen
On game_mode% GoTo beginner, intermediate, expert

beginner:
   mines%  = 10 : Print "Beginner (10 mines)"  : GoTo setup_done
intermediate:
   mines%  = 20 : Print "Intermediate (20 mines)" : GoTo setup_done
expert:
   mines%  = 40 : Print "Expert (40 mines)"
setup_done:
```

#### Select Case

`Select Case` / `End Select` dispatches on a scalar expression. Each `Case` arm is tested
in order; the first matching arm executes and control passes to `End Select`. `Case Else`
is the optional catch-all.

```basic
Select Case score%
   Case Is >= 90
      grade$ = "A"
   Case Is >= 75
      grade$ = "B"
   Case Is >= 60
      grade$ = "C"
   Case Else
      grade$ = "F"
End Select
```

Supported arm forms:

| Form | Example | Matches when… |
| --- | --- | --- |
| Single value | `Case 1` | expression = 1 |
| Value list | `Case 1, 3, 5` | expression is one of the listed values |
| Range | `Case 1 To 10` | 1 ≤ expression ≤ 10 |
| Comparison | `Case Is > 100` | expression satisfies the comparison |
| Fallback | `Case Else` | no earlier arm matched |

Works with integers, doubles, and strings. `Select Case` blocks may be nested.

```basic
' String dispatch
Select Case cmd$
   Case "quit", "exit", "q"
      Print "Goodbye" : End
   Case "help", "?"
      Print "Type a command"
   Case Else
      Print "Unknown: "; cmd$
End Select
```

### 4.4 Subroutines and Functions

nuBASIC supports two kinds of named procedures: `Sub` (no return value) and `Function`
(returns a value). Both accept typed parameters, create their own local variable scope, and
can call themselves recursively. Unlike `GoSub` subroutines, `Sub` and `Function` do not share
variables with the caller — each invocation has its own private set of locals.

#### Sub — a procedure with no return value

A `Sub` groups a set of related statements under a name. It receives its inputs as parameters
and operates on them or on global variables. `Exit Sub` provides an early return path.

```basic
Sub ClearArea(x1%, y1%, x2%, y2%)
   FillRect x1%, y1%, x2%, y2%, &h000000
   Rect     x1%, y1%, x2%, y2%, &h404040
End Sub

Sub PrintCentered(msg$, row%, color%)
   col% = (80 - Len(msg$)) \ 2
   Locate row%, col%
   Print msg$
End Sub

' Calls:
ClearArea 0, 0, 640, 480
PrintCentered "Welcome to nuBASIC", 12, &hffffff
```

#### Function — a procedure that returns a value

A `Function` computes and returns a single value. The return type is indicated either by
the suffix on the function name or by an explicit `As Type` clause. The return value is
set by assigning to the function's own name inside the body.

```basic
Function Factorial%(n%)
   If n% <= 1 Then
      Factorial% = 1
   Else
      Factorial% = n% * Factorial%(n% - 1)   ' recursive call
   End If
End Function

Function Clamp(value, minVal, maxVal)
   If value < minVal Then
      Clamp = minVal
   ElIf value > maxVal Then
      Clamp = maxVal
   Else
      Clamp = value
   End If
End Function

Function Greeting$(name$)
   Greeting$ = "Hello, " + name$ + "! Welcome to nuBASIC."
End Function

' Calls:
Print Factorial%(12)
Print Clamp(1.5, 0.0, 1.0)     ' 1.0
Print Greeting$("World")
```

Functions can return strings, doubles, integers, or any other type. They can also return arrays
by value. `Exit Function` exits early, leaving the last value assigned to the function name as
the return value.

#### ByRef and ByVal parameter qualifiers

By default every parameter is passed **by value** (`ByVal`): the Sub or Function receives a
copy of the caller's value, so mutations inside the procedure have no effect on the caller.
Prefix a parameter with `ByRef` to pass **by reference**: the callee operates directly on the
caller's variable, and any assignment is visible to the caller after the call returns.

Both scalars and `Struct` variables can be passed `ByRef`:

```basic
Sub Swap(ByRef a% As Integer, ByRef b% As Integer)
   Dim tmp% As Integer
   tmp% = a%
   a% = b%
   b% = tmp%
End Sub

Dim x% As Integer, y% As Integer
x% = 7 : y% = 42
Call Swap(x%, y%)
Print x%, y%   ' prints: 42   7

' -------------------------------------------------------
Struct Point
   x As Double
   y As Double
End Struct

Sub Translate(ByRef p As Point, dx As Double, dy As Double)
   p.x = p.x + dx
   p.y = p.y + dy
End Sub

Dim pt As Point
pt.x = 10.0 : pt.y = 20.0
Call Translate(pt, 3.0, -5.0)
Print pt.x, pt.y   ' prints: 13   15
```

When `ByVal` is explicit the parameter name makes the intent clear in the source:

```basic
Sub ShowDouble(ByVal n As Integer)
   n = n * 2
   Print n         ' prints the doubled value
End Sub

Dim v% As Integer
v% = 5
Call ShowDouble(v%)
Print v%            ' still 5 — caller's copy unchanged
```

#### Call keyword

`Call` is an optional keyword that can precede any Sub or Function invocation. When `Call`
is used, the argument list must be enclosed in parentheses:

```basic
Call ClearArea(0, 0, 640, 480)         ' same as: ClearArea 0, 0, 640, 480
Call PrintCentered("Hello", 12, &hfff) ' same as: PrintCentered "Hello", 12, &hfff
```

Both forms are equivalent. `Call` improves readability and is familiar to programmers coming
from Visual Basic or other BASIC dialects.

#### Include directive

The `Include` (or `#Include`) directive loads and executes another source file at the point
where the directive appears. This makes it straightforward to split a program across multiple
files or to share common library routines:

```basic
' main.bas
Include "utils.bas"
Include "graphics.bas"

Call DrawBorder(0, 0, 639, 479)
```

```basic
' utils.bas
Sub DrawBorder(x1%, y1%, x2%, y2%)
   Rect x1%, y1%, x2%, y2%, &hffffff
End Sub
```

The file path is resolved relative to the directory containing the file that issues the
directive. `Include` is processed at load time, so all definitions in the included file are
available to the rest of the including file.

#### Open-ended Array Parameters

A Sub or Function can accept an array of any size by declaring a parameter with empty
parentheses — `param() As Type`. This removes the need to fix the dimension in the
signature and enables generic array-processing routines:

```basic
Sub SumArray(nums() As Integer, ByRef total% As Integer)
   total% = 0
   Dim i% As Integer
   For i% = 0 To SizeOf(nums) - 1
      total% = total% + nums(i%)
   Next i%
End Sub

Dim data%(4)
data%(0) = 10 : data%(1) = 20 : data%(2) = 30 : data%(3) = 40 : data%(4) = 50
Dim s% As Integer
Call SumArray(data%(), s%)
Print s%   ' 150
```

The caller passes the array with its `()` suffix; the callee accesses elements with the
usual index notation.

---

### 4.5 Classes and Objects

Classes group related data (fields) and behaviour (methods) into a single named type.
Declare with `Class`/`End Class`; instantiate with `Dim`.

```basic
Class Rectangle
   Public width  As Double
   Public height As Double

   Function Area() As Double
      Area = Me.width * Me.height
   End Function

   Function Perimeter() As Double
      Perimeter = 2.0 * (Me.width + Me.height)
   End Function
End Class

Dim r As Rectangle
r.width  = 8.0
r.height = 5.0
Print "Area ="; r.Area()        ' 40
Print "Perimeter ="; r.Perimeter()  ' 26
```

`Me` is an implicit reference to the current instance inside every instance method.

#### Static Methods

`Static Function` and `Static Sub` inside a `Class` body are class-level procedures. They
do not receive an implicit instance; they cannot use `Me`. Call them with
`ClassName.Method(args)` — no instance is required:

```basic
Class MathHelper
   Static Function Add(a As Integer, b As Integer) As Integer
      Add = a + b
   End Function

   Static Sub PrintProduct(a As Integer, b As Integer)
      Print "Product ="; a * b
   End Sub
End Class

Dim result% As Integer
result% = MathHelper.Add(3, 7)    ' 10
MathHelper.PrintProduct 4, 5       ' Product = 20
```

Static and instance methods may coexist in the same class. Static methods are ideal for
utility and factory functions that do not need instance state.

---

### 4.6 main() Entry Point

When the top-level program defines a `Function` named `main`, the interpreter calls it as
the program entry point instead of executing statements from the top of the file. The
function must return an `Integer` (the process exit code). Three signatures are supported:

```basic
' Simplest form — no arguments
Function main() As Integer
   Print "Hello from main"
   main = 0
End Function
```

```basic
' Argument count
Function main(argc As Integer) As Integer
   Print "Number of args:"; argc
   main = 0
End Function
```

```basic
' Full argc / argv
Function main(argc As Integer, argv() As String) As Integer
   Dim i% As Integer
   Print "Script: "; argv(0)
   For i% = 1 To argc - 1
      Print "  Arg "; i%; ": "; argv(i%)
   Next i%
   main = 0   ' zero = success
End Function
```

`argc` equals 1 plus the number of extra command-line arguments. `argv(0)` is the script
filename; `argv(1)` through `argv(argc-1)` are the extra arguments supplied after the
filename:

```sh
nubasic -t -e myprog.bas hello world
' → argc = 3
' → argv(0) = "myprog.bas"
' → argv(1) = "hello"
' → argv(2) = "world"
```

When `main` is defined inside an `Include`d file the entry-point behaviour is suppressed in
that file — only the top-level program's `main` is invoked.

---

### 4.7 Namespaced Modules

In **Modern syntax mode** (`Syntax Modern`), the built-in library is organised into named
modules. Functions can be called with a qualified `module::name` prefix. `Syntax Legacy`
(the default) restores the classic flat namespace, so all existing programs work unchanged.

```basic
Syntax Modern

' Qualified calls
Dim s As Double
s = math::sin(0.785398)          ' ≈ 0.7071
Dim h$ As String
h$ = string::left$("Hello", 3)   ' "Hel"
Dim n% As Integer
n% = runtime::sizeof(myArray)
```

#### Using — Import a Module

`Using ModuleName` imports all names from a module into the current scope, removing the
need for the `module::` prefix:

```basic
Syntax Modern
Using math
Using string

Dim v As Double
v = Sin(0.785398)         ' math::sin without prefix
Dim s$ As String
s$ = Left$("World", 3)   ' string::left$ without prefix
```

#### Switching Modes

`Syntax Legacy` may appear anywhere; each `Include`d file honours the mode active at its
include site.

```basic
Syntax Legacy
' Classic global names work again
Print Len("abc")    ' 3
Print Sin(0)        ' 0
```

---

### 4.8 Structures

A `Struct` defines a composite data type that groups several named fields under one name.
Fields can be of any built-in type, and structs can be nested — a field of one struct can be
of another struct type. Once defined, a struct is instantiated with `Dim`, and its fields are
accessed with dot notation.

Structs are especially useful for organising related data that belongs together: coordinates,
bounding boxes, game entities, configuration records.

```basic
Struct Vector2D
   x As Double
   y As Double
End Struct

Struct Sprite
   pos    As Vector2D
   width% As Integer
   height% As Integer
   name$  As String
End Struct

Dim hero As Sprite
hero.pos.x   = 100.0
hero.pos.y   = 200.0
hero.width%  = 32
hero.height% = 48
hero.name$   = "Hero"

Print "Sprite "; hero.name$; " at ("; hero.pos.x; ", "; hero.pos.y; ")"
```

Arrays of structures are declared in the normal way:

```basic
Dim enemies(10) As Sprite
enemies(0).name$ = "Goblin"
enemies(0).pos.x = 50
```

#### Built-in struct types

The interpreter pre-registers two struct types that are available in every program without
any `Struct` definition:

**`DateTime`** — returned by `GetDateTime()`:

```basic
Dim dt As DateTime
dt = GetDateTime()
Print dt.year; "/"; dt.month; "/"; dt.day; "  "; dt.hour; ":"; dt.minute; ":"; dt.second
```

Fields: `year`, `month` (1–12), `day` (1–31), `hour` (0–23), `minute` (0–59),
`second` (0–59), `wday` (0=Sunday), `yday` (0–365).

**`Mouse`** — returned by `GetMouse()` (full build only):

```basic
Dim m As Mouse
m = GetMouse()
If m.btn = 1 Then Print "Left click at "; m.x; ", "; m.y
```

Fields: `x`, `y` (pixels), `btn` (0=none, 1=left, 2=middle, 4=right).

### 4.9 Arrays

Arrays store multiple values of the same type under a single name, accessed by a numeric
index. In nuBASIC, arrays must be declared with `Dim` before use, and the declaration
specifies the maximum index (the array will have `n+1` elements, indexed from `0` to `n`).

```basic
Dim scores%(9)      ' 10 integers, indices 0..9
Dim names$(4)       ' 5 strings, indices 0..4
Dim data(99)        ' 100 doubles, indices 0..99
```

Elements are accessed and assigned using parentheses:

```basic
scores%(0) = 95
scores%(1) = 87
scores%(2) = 72

' Iterate with For
total% = 0
For i% = 0 To 9
   total% = total% + scores%(i%)
Next i%
Print "Average: "; total% / 10
```

`ReDim` resizes an existing array. All current content is discarded and elements are reset to
zero or empty string. Use it when the required size is not known at program start:

```basic
size% = 100
ReDim scores%(size% - 1)   ' now holds size% elements
```

Byte arrays (`@` suffix) are the efficient choice for binary data — file buffers, raw byte
manipulation, image pixel data:

```basic
Dim buf@(1023)        ' 1024-byte buffer
buf@(0) = 65          ' ASCII 'A'
buf@(1) = 66          ' ASCII 'B'
Print Chr$(buf@(0)) + Chr$(buf@(1))   ' AB
```

Multi-dimensional data is handled by declaring a 1-D array and computing the index manually
with the standard row-major formula `row * width + col`:

```basic
Const W% = 10
Dim grid%(W% * W% - 1)   ' 10×10 logical grid

' Access element at (row=2, col=3):
grid%(2 * W% + 3) = 42
Print grid%(2 * W% + 3)
```

### 4.10 Hash Tables

Hash tables (also called dictionaries or associative arrays) map string keys to values of any
type. In nuBASIC a hash table is identified by a name string rather than a variable — the
runtime maintains a global registry of named tables. Any combination of name and key can be
stored and retrieved at any time.

This design makes hash tables particularly useful for configuration stores, lookup tables,
caches, and any scenario where you need to associate a value with a descriptive label rather
than a numeric index.

```basic
' Insert or update entries
HSet "config", "width",     800
HSet "config", "height",    600
HSet "config", "title$",    "My Game"
HSet "config", "fullscreen#", False

' Retrieve values
w% = HGet%("config", "width")
h% = HGet%("config", "height")
t$ = HGet$("config", "title$")
Print t$; " — "; w%; "x"; h%

' Check whether a key exists before reading
If HChk("config", "fullscreen#") Then
   fs# = HGet#("config", "fullscreen#")
   If fs# Then Print "Fullscreen mode"
End If

' Count entries in a table
Print "Config entries: "; HCnt("config")

' Delete a single key
HDel "config", "fullscreen#"

' Delete the entire table (frees all memory)
HDel "config"
```

### 4.11 File I/O

nuBASIC provides three layers of file access: **sequential** (line-oriented text),
**binary** (byte-level access via `FOpen` and `Read#`), and **random access** (seekable).
Files are referred to by an integer file number (`#1`, `#2`, …) that you assign when opening.

#### Sequential text files

Sequential I/O is the simplest model: you open the file for input or output, read or write
lines, and close it when done. The file pointer moves forward automatically with each
`Input#` or `Print#` call.

```basic
' Write a text file
Open "notes.txt" For Output As #1
Print# 1, "First line"
Print# 1, "Second line"
Print# 1, "Score: " + Str$(score%)
Close #1

' Read it back line by line
Open "notes.txt" For Input As #2
While Not(EOF(2))
   Input# 2, line$
   Print line$
Wend
Close #2

' Append without overwriting
Open "notes.txt" For Append As #1
Print# 1, "New entry at " + SysTime$()
Close #1
```

#### Binary and random access

For binary files, `FOpen` accepts C-style mode strings (`"r"`, `"w"`, `"a"`, `"r+"`, …),
giving precise control over read/write/create behaviour. `Read#` reads a fixed number of bytes
directly into a variable, and `Seek` repositions the file pointer.

```basic
' Write raw bytes
FOpen "data.bin", "wb", 1
Dim buf@(3)
buf@(0) = &hDE : buf@(1) = &hAD : buf@(2) = &hBE : buf@(3) = &hEF
Print# 1, buf@
Close #1

' Seek to a known offset and read
FOpen "data.bin", "rb", 2
Seek 2, 2, 2           ' position 2 bytes from start (origin=2)
Read# 2, b@, 1         ' read 1 byte into b@
Print "Byte at offset 2: "; Hex$(b@)
Close #2
```

Useful diagnostic functions — check these after any file operation in production code:

```basic
pos%  = FTell(1)    ' current byte offset
size% = FSize(1)    ' total file size in bytes
err%  = FError(1)   ' non-zero if an error occurred on this file
at_end% = EOF(1)    ' 1 if the file pointer is at the end
code% = Errno()     ' system error code from the last failed operation
Print Errno$(code%) ' human-readable error description
```

#### File system operations

```basic
MkDir("saves")            ' create a directory
RmDir("saves/old")        ' remove an empty directory
Erase("temp.txt")         ' delete a file
Print Pwd$()              ' print current working directory
ChDir ".."                ' change to parent directory
```

### 4.12 DATA, READ, RESTORE

`Data`, `Read`, and `Restore` implement a classic BASIC mechanism for embedding structured
constant tables directly in the source code. Data items are stored in a sequential store when
the `Data` statements execute; `Read` retrieves them in order; `Restore` resets or repositions
the read pointer.

This is a natural fit for look-up tables, sprite definitions, level maps, musical notes, and
any other fixed dataset that would otherwise need an external file or a long series of
assignment statements.

```basic
' Embed a color palette
Data "red",    &hFF0000
Data "green",  &h00FF00
Data "blue",   &h0000FF
Data "yellow", &hFFFF00
Data "white",  &hFFFFFF

' Read and use the palette
For i% = 0 To 4
   Read name$, color%
   FillRect i%*60, 10, i%*60+50, 60, color%
   TextOut  i%*60+5, 70, name$, &hFFFFFF
Next i%

' Rewind and read again from the beginning
Restore
Read first_name$, first_color%
Print "First color: "; first_name$

' Jump to a specific position in the store
Restore 2        ' skip to the third item (index 2, zero-based)
Read name$, color%
Print "Third color: "; name$

' Clear the entire data store
Restore -1
```

A common pattern is to use `Data` to drive a state machine or to initialise arrays at startup,
avoiding hard-coded index assignments scattered through the code.

### 4.13 String Handling

Strings in nuBASIC are variable-length sequences of characters. All source-code string literals
are stored as UTF-8, and the GDI console renders them correctly for any script that the
installed font supports. The standard string functions operate on character positions; most use
1-based indexing (following the BASIC tradition), but `SubStr$` and `PStr$` are 0-based (for
consistency with the underlying byte array model).

#### Measuring and slicing

```basic
s$ = "Hello, World!"

Print Len(s$)           ' 13 — number of characters
Print Left$(s$, 5)      ' "Hello"
Print Right$(s$, 6)     ' "World!"
Print Mid$(s$, 8, 5)    ' "World"   (1-based: start at position 8)
Print SubStr$(s$, 7, 5) ' "World"   (0-based: start at offset 7)
```

#### Searching

`InStr` is case-insensitive; `InStrCS` is case-sensitive. Both return a 1-based position,
or −1 if the substring is not found.

```basic
Print InStr("Hello World", "world")     ' 7  (found, case-insensitive)
Print InStr("Hello World", "xyz")       ' -1 (not found)
Print InStrCS("Hello World", "World")   ' 7  (case-sensitive match)
Print InStrCS("Hello World", "world")   ' -1 (case-sensitive, no match)
```

#### Modifying and building

`PStr$` replaces a single character at a 0-based position, returning the modified string. This
is useful for building or patching fixed-width string buffers (as used internally in
`minesHunter.bas` and similar programs):

```basic
board$ = ".........."        ' 10-character board row
board$ = PStr$(board$, 3, "*")   ' "..*......."  — place mine at column 3
board$ = PStr$(board$, 7, "F")   ' "..*.....F."  — flag at column 7
```

#### Case and character conversion

```basic
Print UCase$("hello")    ' "HELLO"
Print LCase$("WORLD")    ' "world"
Print Asc("A")           ' 65  — ASCII/Unicode code point
Print Chr$(65)           ' "A" — character from code point
Print Spc(4)             ' "    " — four spaces
```

#### Numeric conversions

```basic
n% = Val%("42")           ' string to integer
x  = Val("3.14")          ' string to double
Print Str$(3.14)          ' "3.14"
Print Hex$(255)           ' "FF"
Print StrP$(3.14159, 4)   ' "3.142" — rounded to 4 significant digits
```

#### Escape sequences and Unicode output

Standard escape sequences work inside double-quoted strings:

```basic
Print "Column1\tColumn2\tColumn3"    ' tab-separated
Print "Line 1\nLine 2"               ' newline embedded
Print "Alert!\a"                     ' bell character
```

For Unicode characters beyond ASCII, use the `$u` prefix with `\uXXXX` sequences:

```basic
Print $u, "Caf\u00e9 — \u00e0 \u00e8 \u00ec"   ' Café — à è ì
Print $u, "\u03b1 \u03b2 \u03b3"               ' α β γ  (Greek)
Print $u, "\u4e2d\u6587"                        ' 中文   (Chinese)
```

#### Dynamic evaluation

`Eval` parses and evaluates a string as a nuBASIC expression at run time, with access to all
currently defined variables. This allows building expressions programmatically:

```basic
x = 5
expr$ = "x * x + 2 * x + 1"
Print Eval(expr$)       ' 36 — evaluated with x=5
```

---

## 5. Graphics and Multimedia

All graphics functions are available in the full build (Windows GDI or Linux/X11). They are
absent in the *console* build, which targets minimal environments.

The coordinate system has its origin at the **top-left** corner of the working window client
area, with X increasing to the right and Y increasing downward. All coordinates are in pixels.

Colors are packed RGB integers. The most readable way to specify them is hexadecimal with the
`&h` prefix. **Important**: nuBASIC color values follow the GDI byte order (BGR stored in the
low three bytes), so the byte layout is `&hBBGGRR`:

| Color   | nuBASIC constant |
|---------|-----------------|
| Red     | `&h0000FF`      |
| Green   | `&h00FF00`      |
| Blue    | `&hFF0000`      |
| Yellow  | `&h00FFFF`      |
| Cyan    | `&hFFFF00`      |
| Magenta | `&hFF00FF`      |
| White   | `&hFFFFFF`      |
| Black   | `&h000000`      |

The `Rgb(r, g, b)` function computes a color value from separate 0–255 red, green, and blue
components, which is often clearer than a raw hex literal:

```basic
skyBlue%  = Rgb(135, 206, 235)
sunYellow% = Rgb(255, 220, 50)
```

### 5.1 Drawing Primitives

#### Lines and shapes

```basic
' Draw a straight line from (x1,y1) to (x2,y2)
Line x1, y1, x2, y2, color%

' Rectangle outline
Rect x1, y1, x2, y2, color%

' Filled rectangle (the most common primitive — use for backgrounds and clearing regions)
FillRect x1, y1, x2, y2, color%

' Ellipse outline (bounding box given by the two corner points)
Ellipse x1, y1, x2, y2, color%

' Filled ellipse
FillEllipse x1, y1, x2, y2, color%
```

#### Pixels

Individual pixels can be written and read. `SetPixel` is the building block for custom
renderers such as the Mandelbrot demo in `rosetta.bas`.

```basic
SetPixel x%, y%, color%       ' write a pixel
c% = GetPixel(x%, y%)         ' read a pixel's color
```

#### Text on screen

`TextOut` draws a text string at a pixel coordinate using the current console font. Unlike
`Print`, which moves the text cursor and scrolls the buffer, `TextOut` draws directly onto
the graphics surface at a fixed position and does not affect the text cursor.

```basic
TextOut x%, y%, text$, color%

' Examples:
TextOut 10,  10, "Score: " + Str$(score%), &hFFFFFF
TextOut 200, 240, "GAME OVER", Rgb(255, 50, 50)
```

#### Bitmap images

`PlotImage` loads a BMP file from disk and draws it starting at the given top-left pixel
coordinate. The image is drawn at its native size with no scaling.

```basic
PlotImage "background.bmp", 0, 0
PlotImage "sprite.bmp", hero_x%, hero_y%
```

#### Complete scene example

```basic
Cls
MoveWindow GetWindowX(), GetWindowY(), 640, 480
FillRect 0, 0, 640, 480, Rgb(30, 30, 60)        ' dark night sky
FillEllipse 520, 20, 600, 100, Rgb(255, 255, 200) ' moon
FillRect 0, 360, 640, 480, Rgb(0, 80, 0)         ' green ground
For i% = 0 To 5
   cx% = 80 + i% * 100
   FillEllipse cx%, 200, cx%+60, 360, Rgb(20, 120, 20)  ' trees
   FillRect cx%+25, 330, cx%+35, 365, Rgb(80, 50, 20)   ' trunks
Next i%
TextOut 10, 10, "nuBASIC Graphics Demo", &hFFFFFF
```

### 5.2 Flicker-free Rendering

#### The problem

When a program calls multiple drawing commands in sequence — clearing the background, drawing
the board, drawing the score, drawing the player sprite — each individual call immediately
blits the back buffer to the visible screen. The user briefly sees each intermediate state:
a cleared black frame, then a partial board, then the board with the score, and finally the
complete frame. This manifests as visible flicker, particularly at high frame rates or with
complex scenes.

#### The solution: double buffering

`ScreenLock` tells the renderer to accumulate all subsequent drawing into an off-screen back
buffer without presenting anything to the screen. When `ScreenUnlock` is called, the complete
back buffer is transferred to the screen in a single, atomic operation — the user sees only
the finished frame, never the intermediate states. `Refresh` performs the same blit on demand
without changing the lock state.

| Instruction | Effect |
|-------------|--------|
| `ScreenLock` | Suspend automatic screen refresh; all drawing goes to the back buffer |
| `ScreenUnlock` | Present the back buffer to the screen in one blit; resume automatic refresh |
| `Refresh` | Force an immediate blit; lock state is unchanged |

#### Pattern 1 — game frame loop

This is the canonical pattern for any animated program. Every frame is composed entirely on
the back buffer and presented as a single unit:

```basic
While Not(game_over%)
   ScreenLock
   FillRect 0, 0, 640, 480, &h000000   ' clear: erase previous frame
   DrawBoard                            ' render game board
   DrawPlayer player_x%, player_y%      ' render player
   DrawEnemies                          ' render all enemies
   DrawHUD score%, lives%               ' render heads-up display
   ScreenUnlock
   MDelay 16                            ' pace to ~60 fps
Wend
```

#### Pattern 2 — progressive rendering (Mandelbrot / fractal)

For computationally expensive renders where you want to show progress, use `ScreenLock`
at the outer loop level so each column (or row) is presented atomically rather than pixel
by pixel:

```basic
For x0 = -2 To 2 Step 0.013
   ScreenLock
   For y0 = -1.5 To 1.5 Step 0.013
      ' ... compute iteration count c ...
      FillRect x0*d%+dx%, y0*d%+dy%, x0*d%+dx%+2, y0*d%+dy%+2, c%*16
   Next y0
   ScreenUnlock       ' one column appears per iteration — smooth progressive reveal
Next x0
```

#### Pattern 3 — atomic erase + redraw (moving object)

To move an object without ghost trails, erase the old position and draw the new position
inside a single lock:

```basic
' Compute new position
x_old% = x%  :  y_old% = y%
x% = x% + dx%  :  y% = y% + dy%

ScreenLock
FillEllipse x_old%*10, y_old%*10, x_old%*10+10, y_old%*10+10, 0        ' erase old
FillEllipse x%*10,     y%*10,     x%*10+10,     y%*10+10,     &hffffff ' draw new
ScreenUnlock
```

#### Pattern 4 — dialog before blocking input

Before waiting for a key with `Input$()`, use `Refresh` to ensure the dialog is visible.
Without it, if a `ScreenLock` is active elsewhere in the program, the user might not see
the dialog:

```basic
ScreenLock
FillRect 150, 220, 490, 310, &hffff00
Rect     150, 220, 490, 310, &h000000
TextOut  180, 245, "Game over! Play again? (Y/N)", &h000000
ScreenUnlock
Refresh
key$ = Input$(1)
```

### 5.3 Mouse Input

nuBASIC programs can read the mouse position and button state at any point. Mouse input is
polled — there is no event queue — so typical usage is to call `GetMouse()` once per frame
inside the main loop and act on the struct fields returned.

#### GetMouse() — preferred

`GetMouse()` captures position and button state in a single call and returns a `Mouse` struct.
The `Mouse` type is pre-registered by the interpreter, so no `Struct` definition is needed:

```basic
Dim m As Mouse
m = GetMouse()
' m.x   — cursor X in pixels from left edge of window
' m.y   — cursor Y in pixels from top edge of window
' m.btn — 0=none, 1=left, 2=middle, 4=right
```

#### Legacy scalar functions *(deprecated — removed in v2.0)*

```basic
btn% = GetMouseBtn()   ' use GetMouse().btn instead
x%   = GetMouseX()     ' use GetMouse().x   instead
y%   = GetMouseY()     ' use GetMouse().y   instead
```

#### Hit-testing a button region

The most common use of mouse input is testing whether the cursor is inside a rectangular
region at the moment a button is pressed:

```basic
' Define a button rectangle
bx1% = 40  :  by1% = 60
bx2% = 200 :  by2% = 100

' Draw the button
FillRect bx1%, by1%, bx2%, by2%, &hffff00
TextOut bx1%+20, by1%+15, "Click me", &h000000

' Main interaction loop
Dim m As Mouse
While 1
   m = GetMouse()

   If m.btn = 1 And m.x >= bx1% And m.x <= bx2% And m.y >= by1% And m.y <= by2% Then
      Print "Button clicked!"
      MDelay 200    ' debounce — wait for release
   End If

   MDelay 16
Wend
```

#### Drawing with the mouse

```basic
Cls
FillRect 0, 0, 640, 480, &h000000
TextOut 10, 10, "Hold left button and draw. Press Q to quit.", &hffffff

Dim m As Mouse
While 1
   key$ = InKey$()
   If key$ = "q" Or key$ = "Q" Then Exit While

   m = GetMouse()
   If m.btn = 1 Then
      SetPixel m.x, m.y, &hffffff
   End If

   MDelay 5
Wend
```

### 5.4 Sound and Window Management

#### Window control

The nuBASIC working window can be positioned and sized from BASIC code. This is useful for
setting up the correct canvas size at the start of a graphical program, or for implementing
full-screen-style layouts.

```basic
' Resize and position the window
MoveWindow GetWindowX(), GetWindowY(), 800, 600

' Query current window geometry
Print "Position: "; GetWindowX(); ", "; GetWindowY()
Print "Size:     "; GetWindowDx(); " x "; GetWindowDy()

' Query the drawable client area (may differ from window size due to title bar / borders)
Print "Canvas: "; GetSWidth(); " x "; GetSHeight()

' Keep the window above all others
SetTopMost()
```

A typical program startup sequence:

```basic
Cls
MoveWindow 100, 100, 640, 480
FillRect 0, 0, 640, 480, &h000000   ' clear to black after resize
```

#### Sound

`PlaySound` plays a WAV audio file. The second argument controls synchronous vs asynchronous
playback: 0 blocks until the sound finishes, 1 returns immediately so the program continues
running while the sound plays.

```basic
PlaySound "background.wav", 1   ' async: music plays in background
PlaySound "explosion.wav", 0    ' sync: wait for it to finish
PlaySound "coin.wav", 1
```

`Beep` produces the system default beep tone — useful for simple feedback in games or as
an error indicator:

```basic
Beep   ' simple system beep
```

#### Message boxes

`MsgBox` displays a modal dialog that halts program execution until the user dismisses it.
It is useful for alerts, error messages, and simple yes/no confirmation prompts.

```basic
result% = MsgBox("nuBASIC Demo", "Setup complete. Ready to play?")
If result% > 0 Then
   Print "User confirmed."
End If
```

---

## 6. Command Reference

### 6.1 Console Commands

These commands are available in the interactive REPL. They control the interpreter itself —
loading, saving, debugging, and inspecting programs — and are not valid inside a running
BASIC program.

| Command | Description | Syntax |
|---------|-------------|--------|
| `Run` | Execute the program in memory | `Run [linenumber]` |
| `Cont` | Continue after a `Stop` or breakpoint | `Cont` |
| `Resume` | Resume from the next line after interrupt | `Resume` |
| `Break` | Set a breakpoint, optionally conditional | `Break [[linenum] [If expr]]` |
| `RmBrk` | Remove a breakpoint | `RmBrk linenum` |
| `ClrBrk` | Remove all breakpoints | `ClrBrk` |
| `Load` | Load a program file into memory | `Load filename` |
| `Save` | Save the current program to a file | `Save filename` |
| `Exec` | Load and run a program file | `Exec filename` |
| `List` | List all or part of the program | `List [from[-to]]` |
| `New` | Clear program and all variables | `New` |
| `Clr` | Clear all variables (keep program) | `Clr` |
| `Renum` | Renumber program lines | `Renum [increment]` |
| `Grep` | List lines matching a pattern | `Grep pattern` |
| `Vars` | Show current run-time variables | `Vars` |
| `Meta` | Show build metadata | `Meta` |
| `Ver` | Print interpreter version | `Ver` |
| `TrOn` | Enable program tracing | `TrOn` |
| `TrOff` | Disable program tracing | `TrOff` |
| `StOn` | Enable step-by-step execution | `StOn` |
| `StOff` | Disable step-by-step execution | `StOff` |
| `Pwd` | Print current working directory | `Pwd` |
| `Cd` | Change working directory | `Cd path` |
| `Help` | Show help for a keyword | `Help [keyword]` |
| `Apropos` | Search help descriptions | `Apropos keyword` |
| `!` | Execute a shell command | `! shell_command` |
| `Exit` | Quit the interpreter | `Exit` |

### 6.2 Instructions

#### Program structure

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `Dim` | `Dim var[(n)] [As Type]` | Declare variable or array |
| `ReDim` | `ReDim var(n)` | Resize array (clears content) |
| `Const` | `Const name [As Type] = value` | Declare a constant |
| `Let` | `[Let] var = expr` | Assign a value (keyword optional) |
| `Sub` | `Sub name(params)` … `End Sub` | Define a subroutine |
| `Function` | `Function name(params) [As Type]` … `End Function` | Define a function |
| `Struct` | `Struct name` … `End Struct` | Define a composite type |
| `End` | `End` | Stop program execution |
| `Stop` | `Stop` | Break into REPL (resume with `Cont`) |

#### Control flow

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `If` | `If expr Then … [ElIf expr Then …] [Else …] End If` | Conditional branch |
| `For` | `For var=x To y [Step z]` … `Next [var]` | Counted loop |
| `While` | `While cond` … `Wend \| End While` | Pre-condition loop |
| `Do` | `Do` … `Loop While cond` | Post-condition loop |
| `Exit For` | `Exit For` | Break out of For loop |
| `Exit While` | `Exit While` | Break out of While loop |
| `Exit Do` | `Exit Do` | Break out of Do loop |
| `Exit Sub` | `Exit Sub` | Return from Sub early |
| `Exit Function` | `Exit Function` | Return from Function early |
| `GoTo` | `GoTo label \| linenum` | Unconditional jump |
| `GoSub` | `GoSub label \| linenum` | Call a subroutine |
| `Return` | `Return` | Return from GoSub |
| `On` | `On expr GoTo label1[, label2, …]` | Computed branch |

#### I/O

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `Print` | `Print [expr[; \| , …]]` | Output to console |
| `Write` | `Write [expr[; \| , …]]` | Output without trailing newline |
| `Print#` | `Print# filenum, expr` | Output to file |
| `Input` | `Input [prompt;] var` | Read from console |
| `Input#` | `Input# filenum, var` | Read from file |
| `Read#` | `Read# filenum, var, size` | Raw binary read |
| `Open` | `Open filename For mode As #n` | Open a file |
| `FOpen` | `FOpen filename, mode$, n` | Open a file (C-style mode string) |
| `Close` | `Close #n` | Close a file |
| `Seek` | `Seek filenum, pos, origin` | Move file pointer |
| `Flush` | `Flush filenum` | Flush file buffer to disk |
| `Locate` | `Locate row, col` | Move text cursor (1-based) |
| `Cls` | `Cls` | Clear the console screen |
| `Beep` | `Beep` | Sound the system beep |

#### Timing

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `Delay` / `Sleep` | `Delay seconds` | Pause execution (in seconds) |
| `MDelay` | `MDelay milliseconds` | Pause execution (in milliseconds) |

#### Data

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `Data` | `Data expr1[, expr2, …]` | Store constant data values |
| `Read` | `Read var1[, var2, …]` | Read from the Data store |
| `Restore` | `Restore [index]` | Rewind or reposition Data pointer |

#### Graphics

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `Line` | `Line x1,y1,x2,y2,color` | Draw a line |
| `Rect` | `Rect x1,y1,x2,y2,color` | Draw rectangle outline |
| `FillRect` | `FillRect x1,y1,x2,y2,color` | Draw filled rectangle |
| `Ellipse` | `Ellipse x1,y1,x2,y2,color` | Draw ellipse outline |
| `FillEllipse` | `FillEllipse x1,y1,x2,y2,color` | Draw filled ellipse |
| `SetPixel` | `SetPixel x,y,color` | Draw a single pixel |
| `TextOut` | `TextOut x,y,text$,color` | Draw text at pixel coordinates |
| `PlotImage` | `PlotImage bitmap$,x,y` | Draw a BMP image |
| `ScreenLock` | `ScreenLock` | Suspend screen refresh (begin compositing frame) |
| `ScreenUnlock` | `ScreenUnlock` | Present back buffer to screen (end frame) |
| `Refresh` | `Refresh` | Force immediate blit (lock state unchanged) |

#### Miscellaneous

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `ChDir` | `ChDir path$` | Change working directory |
| `Shell` | `Shell "command"` | Execute a shell command |
| `Randomize` | `Randomize` | Seed the random number generator with current time |

### 6.3 Built-in Functions

#### Math

| Function | Returns | Description |
|----------|---------|-------------|
| `Abs(x)` | Double | Absolute value |
| `Int(x)` | Integer | Truncate toward zero |
| `TruncF(x)` | Double | Round toward zero (double result) |
| `Sign(x)` | Integer | −1, 0, or 1 |
| `Sqr(x)` / `Sqrt(x)` | Double | Square root |
| `Pow(x,y)` | Double | x raised to the power y |
| `Exp(x)` | Double | e^x |
| `Log(x)` | Double | Natural logarithm |
| `Log10(x)` | Double | Base-10 logarithm |
| `Sin(x)` | Double | Sine (x in radians) |
| `Cos(x)` | Double | Cosine (x in radians) |
| `Tan(x)` | Double | Tangent (x in radians) |
| `ASin(x)` | Double | Arc sine (result in radians) |
| `ACos(x)` | Double | Arc cosine (result in radians) |
| `ATan(x)` | Double | Arc tangent (result in radians) |
| `Sinh(x)` | Double | Hyperbolic sine |
| `Cosh(x)` | Double | Hyperbolic cosine |
| `Tanh(x)` | Double | Hyperbolic tangent |
| `PI()` | Double | π ≈ 3.14159265358979 |
| `Rnd(x)` | Double | Random number in [0,1); x<0 reseeds the generator |
| `Min(x,y)` | Double | Smaller of x and y |
| `Max(x,y)` | Double | Larger of x and y |
| `Conv(v1,v2[,c1,c2])` | Array | Discrete convolution of two numeric vectors |

#### String

| Function | Returns | Description |
|----------|---------|-------------|
| `Len(s$)` | Integer | Length in characters |
| `Left$(s$,n)` | String | First n characters |
| `Right$(s$,n)` | String | Last n characters |
| `Mid$(s$,n,m)` | String | m characters from 1-based position n |
| `SubStr$(s$,n,m)` | String | m characters from 0-based offset n |
| `PStr$(s$,pos,c$)` | String | Return s$ with character at 0-based pos replaced by c$ |
| `InStr(s$,t$)` | Integer | 1-based position of t$ in s$; −1 if absent (case-insensitive) |
| `InStrCS(s$,t$)` | Integer | Same but case-sensitive |
| `UCase$(s$)` | String | Convert to upper case |
| `LCase$(s$)` | String | Convert to lower case |
| `Str$(x)` | String | Number to string |
| `StrP$(x,p%)` | String | Number to string with p% significant digits |
| `Hex$(x)` | String | Hexadecimal representation |
| `Val(s$)` | Double | Parse string as floating-point number |
| `Val%(s$)` | Integer | Parse string as integer |
| `Asc(s$)` | Integer | Unicode code point of first character |
| `Chr$(n)` | String | Character from Unicode code point |
| `Spc(n)` | String | String of n spaces |
| `Eval(s$)` | Any | Evaluate a nuBASIC expression string at run time |

#### Keyboard / Input

| Function | Returns | Description |
|----------|---------|-------------|
| `InKey$()` | String | Non-blocking key read; empty string if no key is waiting |
| `Input$(n)` | String | Block until exactly n characters are read |
| `GetVKey()` | Integer | Read next virtual key code from input |

#### File

| Function | Returns | Description |
|----------|---------|-------------|
| `EOF(n)` | Integer | 1 if the file pointer is at end-of-file; 0 otherwise |
| `FError(n)` | Integer | Non-zero if an error has occurred on file n |
| `FTell(n)` | Integer | Current byte offset in file n |
| `FSize(n)` | Integer | Total size in bytes of file n |
| `Errno()` | Integer | System error code from the last failed operation |
| `Errno$(n)` | String | Human-readable description for error code n |
| `MkDir(path$)` | Integer | Create a directory; returns 0 on success |
| `RmDir(path$)` | Integer | Remove a directory; returns 0 on success |
| `Erase(path$)` | Integer | Delete a file; returns 0 on success |
| `Pwd$()` | String | Current working directory |
| `GetEnv$(var$)` | String | Value of an environment variable |
| `SetEnv(var$,val$)` | Integer | Set an environment variable |
| `UnsetEnv(var$)` | Integer | Unset an environment variable |

#### System / Time

| Function | Returns | Description |
|----------|---------|-------------|
| `GetDateTime()` | DateTime struct | All date/time fields: `year`, `month`, `day`, `hour`, `minute`, `second`, `wday`, `yday` |
| `SysTime$()` | String | Current local time and date as a string |
| `Time()` | Integer | Seconds elapsed since the Unix Epoch |
| `SysHour()` | Integer | Current hour (0–23) *(deprecated v2.0 — use `GetDateTime().hour`)* |
| `SysMin()` | Integer | Current minute (0–59) *(deprecated v2.0 — use `GetDateTime().minute`)* |
| `SysSec()` | Integer | Current second (0–59) *(deprecated v2.0 — use `GetDateTime().second`)* |
| `SysDay()` | Integer | Day of the month (1–31) *(deprecated v2.0 — use `GetDateTime().day`)* |
| `SysMonth()` | Integer | Month (1–12) *(deprecated v2.0 — use `GetDateTime().month`)* |
| `SysYear()` | Integer | Full year (e.g. 2026) *(deprecated v2.0 — use `GetDateTime().year`)* |
| `SysWDay()` | Integer | Day of week (0=Sunday … 6=Saturday) *(deprecated v2.0 — use `GetDateTime().wday`)* |
| `SysYDay()` | Integer | Day of year (0–365) *(deprecated v2.0 — use `GetDateTime().yday`)* |
| `GetPlatId()` | Integer | 1 = Windows, 2 = Linux/other |
| `GetAppPath$()` | String | Full path of the nuBASIC executable |
| `Ver$()` | String | nuBASIC version string |

#### Graphics / Window (full build only)

| Function | Returns | Description |
|----------|---------|-------------|
| `Rgb(r,g,b)` | Integer | Compose an RGB color from 0–255 components |
| `GetPixel(x,y)` | Integer | Read the color of a pixel |
| `GetMouse()` | Mouse struct | All pointer state: `x`, `y`, `btn` |
| `GetMouseX()` | Integer | Mouse cursor X in pixels *(deprecated v2.0 — use `GetMouse().x`)* |
| `GetMouseY()` | Integer | Mouse cursor Y in pixels *(deprecated v2.0 — use `GetMouse().y`)* |
| `GetMouseBtn()` | Integer | Mouse button bitmask (1=left, 2=middle, 4=right) *(deprecated v2.0 — use `GetMouse().btn`)* |
| `GetSWidth()` | Integer | Width of the drawable client area in pixels |
| `GetSHeight()` | Integer | Height of the drawable client area in pixels |
| `GetWindowX()` | Integer | Window left edge position on screen |
| `GetWindowY()` | Integer | Window top edge position on screen |
| `GetWindowDx()` | Integer | Total window width in pixels |
| `GetWindowDy()` | Integer | Total window height in pixels |
| `MoveWindow(x,y,w,h)` | Integer | Move and resize the working window |
| `SetTopMost()` | Integer | Make the window always-on-top |
| `MsgBox(title$,msg$)` | Integer | Show a modal dialog; returns > 0 if confirmed |
| `PlaySound(file$,async%)` | Integer | Play a WAV file (async%=1 returns immediately) |

#### Hash Tables

| Function | Returns | Description |
|----------|---------|-------------|
| `HSet(hash$,key$,val)` | — | Insert or update an entry |
| `HGet(hash$,key$)` | Any | Retrieve the value for a key |
| `HChk(hash$,key$)` | Boolean | True if the key exists in the table |
| `HCnt(hash$)` | Integer | Number of entries in the table |
| `HDel(hash$[,key$])` | — | Delete a single key, or the whole table if key omitted |

#### Miscellaneous

| Function | Returns | Description |
|----------|---------|-------------|
| `Not(x)` | Integer | Logical NOT: 1 if x=0, 0 otherwise |
| `bNot(x)` | Integer | Bitwise NOT of x |
| `SizeOf(x)` | Integer | Size in bytes of a variable or entire array |
| `SizeOf@(x)` | Integer | Number of elements in an array |

### 6.4 Operators Reference

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Add numbers or concatenate strings | `a + b`, `"Hi" + name$` |
| `-` | Subtract; unary negation | `a - b`, `-x` |
| `*` | Multiply | `a * b` |
| `/` | Divide (floating-point result) | `a / b` |
| `\` or `Div` | Integer divide (truncates) | `a \ b` |
| `Mod` | Modulo — remainder after integer division | `a Mod b` |
| `^` | Exponentiation | `2 ^ 10` |
| `++` | Increment prefix | `++i%` |
| `--` | Decrement prefix | `--i%` |
| `=` | Equal (comparison) or assignment | `a = b` |
| `<>` | Not equal | `a <> b` |
| `<` `>` `<=` `>=` | Ordered comparison | `a < b` |
| `And` | Logical AND | `a And b` |
| `Or` | Logical OR | `a Or b` |
| `Xor` | Logical exclusive OR | `a Xor b` |
| `Not` | Logical NOT | `Not(x)` |
| `bAnd` | Bitwise AND | `a bAnd b` |
| `bOr` | Bitwise OR | `a bOr b` |
| `bXor` | Bitwise XOR | `a bXor b` |
| `bNot` | Bitwise NOT | `bNot(x)` |
| `bShl` | Shift left | `a bShl 2` |
| `bShr` | Shift right | `a bShr 2` |

---

## 7. Integrated Development Environment (IDE)

The nuBASIC IDE bundles an advanced syntax highlighting editor, an interpreter, and a debugger
into a single application available for Windows and Linux (GTK+2). It is the recommended tool
for writing non-trivial programs — while the CLI screen editor is sufficient for short scripts
and experimentation, the IDE provides the full development experience.

---

### 7.1 Syntax Highlighting Editor

The editor is built on the [Scintilla](https://www.scintilla.org/) library and provides
specialized features for nuBASIC source code.

**Main features:**

- **Syntax highlighting** — nuBASIC keywords, literals, comments, and identifiers are rendered
  in distinct colours, making the structure of a program immediately visible.
- **Code folding** — sections of code can be collapsed to a single summary line and later
  expanded again; fold points are derived from indentation levels.
- **Auto-completion** — pressing `F12` opens a pop-up list of matching keywords; press `Tab`
  to insert the highlighted item.
- **Bookmarks** — any line can be bookmarked with `Ctrl+F7`; bookmark navigation jumps through
  them in forward (`Ctrl+F8`) or reverse (`Ctrl+F9`) order.
- **Find and Replace** — full-text search with optional case sensitivity, whole-word matching,
  and regular-expression replacement.
- **Context-sensitive help** — select a keyword and press `F1` to display the built-in help
  entry for that keyword directly from the interpreter; press `Ctrl+F1` to open the online help
  in a browser.
- **Integrated debugger** — breakpoints, step-by-step execution, expression evaluation
  (data-tips), and program-counter tracking are all accessible without leaving the editor.
- **Line and column indicator** — the toolbar permanently shows the cursor position.
- **Zoom** — `F3` zooms in; `Ctrl+F3` zooms out.

---

### 7.2 Keyboard Commands

Editor movement keys (`Arrow`, `Page Up/Down`, `Home`, `End`) extend a stream selection when
`Shift` is held, and extend a rectangular selection when both `Shift` and `Alt` are held.
Keyboard equivalents for menu commands are listed in the menus. Additional commands with no
menu equivalent are:

| Action | Key |
|---|---|
| Indent block | `Tab` |
| Dedent block | `Shift+Tab` |
| Delete to start of word | `Ctrl+Backspace` |
| Delete to end of word | `Ctrl+Delete` |
| Delete to start of line | `Ctrl+Shift+Backspace` |
| Delete to end of line | `Ctrl+Shift+Delete` |
| Go to start of document | `Ctrl+Home` |
| Extend selection to start of document | `Ctrl+Shift+Home` |
| Go to start of display line | `Alt+Home` |
| Go to end of document | `Ctrl+End` |
| Extend selection to end of document | `Ctrl+Shift+End` |
| Go to end of display line | `Alt+End` |
| Scroll up | `Ctrl+Up` |
| Scroll down | `Ctrl+Down` |
| Line cut | `Ctrl+L` |
| Line delete | `Ctrl+Shift+L` |
| Duplicate selection | `Ctrl+D` |
| Previous paragraph (Shift extends selection) | `Ctrl+[` |
| Next paragraph (Shift extends selection) | `Ctrl+]` |
| Previous word (Shift extends selection) | `Ctrl+Left` |
| Next word (Shift extends selection) | `Ctrl+Right` |
| Previous word part (Shift extends selection) | `Ctrl+/` |
| Next word part (Shift extends selection) | `Ctrl+\` |
| Rectangular block selection | `Alt+Shift+Movement` |
| Extend rectangular selection to start of line | `Alt+Shift+Home` |
| Extend rectangular selection to end of line | `Alt+Shift+End` |
| Go to line… | `F2` |
| Zoom in | `F3` |
| Zoom out | `Ctrl+F3` |

---

### 7.3 Code Folding

Program source code has a natural hierarchy of sections and sub-sections. The folding feature
lets you hide any section, replacing it with a single overview line, and reveal it again later.
Fold points are determined by indentation.

The fold-point markers in the fold margin behave as follows:

- **Click** — expand or collapse that fold without affecting its children. Children that were
  individually folded remain so when the parent is expanded.
- **Ctrl+Click** — toggle the fold and apply the same operation recursively to all children.
- **Shift+Click** — expand all child folds without toggling the clicked fold itself.
- **Ctrl+Shift+Click anywhere in the fold margin** — expand or contract all top-level folds
  simultaneously. The **View → Toggle all folds** menu item does the same.

> **Tip:** To open a large block with all grandchildren still folded, first collapse it with
> `Ctrl+Click`, then expand it with a plain click. Child folds will be visible but still
> collapsed; open them individually as needed.

---

### 7.4 Find and Replace

**Find (`Ctrl+F` or Search → Find…)**

Opens the Find dialog. Enter the text to locate in the *Find what* box (the current selection
pre-fills this box). Options:

- **Direction** — search upward toward the start of the document or downward toward the end.
- **Match whole word only** — matches only when the string is surrounded by spaces, tabs,
  punctuation, or line boundaries.
- **Match case** — case-sensitive search; when unchecked the search is case-insensitive.
- **Find Next** — moves to the next occurrence.

**Find and Replace (`Ctrl+R` or Search → Find and Replace…)**

Adds a *Replace with* box to the above, plus:

- **Replace** — replace the current match and advance to the next.
- **Replace All** — replace every occurrence in one operation.
- Regular-expression replacement is supported.

---

### 7.5 Bookmarks

A bookmark is a persistent marker attached to a source line. Bookmarked lines are indicated by
a circle in the left margin.

| Menu command | Shortcut | Description |
|---|---|---|
| Add/Remove marker | `Ctrl+F7` | Toggle a bookmark on the current line |
| Find next marker | `Ctrl+F8` | Jump to the next bookmark |
| Find previous marker | `Ctrl+F9` | Jump to the previous bookmark |
| Remove all markers | — | Clear every bookmark in the current document |

All bookmark commands are also available in the **Search** menu.

---

### 7.6 Auto-Completion

Pressing `F12` opens a pop-up list populated with nuBASIC keywords that match the characters
typed so far. Navigate the list with the arrow keys and press `Tab` to insert the selected
keyword at the cursor position.

---

### 7.7 Toolbar

The toolbar provides one-click access to the most frequent file, debug, and search operations.
It also displays the cursor's current line number and column number, and shows an indicator
when the document has unsaved changes.

| Button | Menu item | Shortcut |
|---|---|---|
| New | File → New | `Ctrl+N` |
| Open | File → Open… | `Ctrl+O` |
| Save | File → Save | `Ctrl+S` |
| Debug | Debug → Start Debugging | `F5` |
| Stop | Debug → Stop Debugging | `Esc` |
| Breakpoint | Debug → Toggle Breakpoint | `F9` |
| Build | Debug → Build Program | `Ctrl+B` |
| Evaluate | Debug → Evaluate Selection | `F11` |
| Step | Debug → Step | `F10` |
| Cont | Debug → Continue Debugging | `F8` |
| Find | Search → Find… | `Ctrl+F` |
| Con Top | Debug → Console Window Topmost | — |
| Ide Top | Debug → IDE Window Topmost | — |

---

### 7.8 Context-Sensitive Help and Online Help

**Context-sensitive help (`F1`)**

Select a keyword in the editor (or position the cursor on it) and press `F1`. The IDE queries
the interpreter's built-in help database and displays the full description of that keyword in
the output panel — the same information returned by `Help <keyword>` in the REPL, without
leaving the editor.

**Online help (`Ctrl+F1`)**

With a keyword selected, pressing `Ctrl+F1` constructs a search query for the keyword and
opens it in the default web browser, displaying the relevant online help topic.

---

### 7.9 Integrated Debugger

The integrated debugger works hand-in-hand with the syntax highlighting editor. Breakpoints
appear as red circles in the left margin; the program-counter arrow marks the next line to
execute; expression results appear as inline annotations directly in the source.

#### Debug Menu

| Command | Shortcut | Description |
|---|---|---|
| Build Program | `Ctrl+B` | Compiles program metadata used for all subsequent executions. Runs automatically the first time the program is started and whenever the source has been modified. |
| Start Debugging | `F5` | Launches the program with the debugger attached. Execution continues until a breakpoint is hit, `Ctrl+C` is pressed, an error occurs, or the program ends normally. |
| Stop Debugging | `Esc` | Terminates the running program immediately. |
| Continue Debugging | `F8` | Resumes execution from the current breakpoint until the next breakpoint or program end. |
| Step | `F10` | Executes the current line and halts at the next. When the line is a `Sub` or `Function` call, execution enters the called routine and halts at its first statement. |
| Evaluate Selection (Data-Tips) | `F11` | Evaluates the selected variable or expression in the context of the halted program and displays the result as an inline annotation next to the selection. Useful for inspecting complex expressions without adding extra `Print` statements. |
| Start Without Debugging | `Ctrl+F5` | Runs the program without the debugger attached. Press `Ctrl+C` during execution to interrupt and return to the CLI prompt (see chapter 3). |
| Toggle Breakpoint | `F9` | Adds or removes a breakpoint on the current line. Active breakpoints are shown as a red circle in the margin. |
| Delete All Breakpoints | — | Removes every breakpoint in the current document. |
| Go to Program Counter | — | Scrolls the editor to the line that will execute next when execution is resumed. |
| Go to Procedure | — | Opens a dynamic sub-menu listing every function and sub-routine in the program. Selecting an entry moves the cursor to that routine's entry point. The list is regenerated each time the program is built. |
| Console Window Topmost | — | Keeps the nuBASIC console window above all other windows, even when it loses focus — useful when running a program that produces graphical output alongside the editor. |
| IDE Window Topmost | — | Returns normal z-order to the console and brings the IDE window to the foreground. |

#### Output Panel

The bottom of the IDE contains a tabbed panel with two tabs:

- **Output** — displays interpreter messages, build results, and evaluation results from
  Data-Tips. Messages are colour-coded: normal output on a white background, warnings on
  yellow, errors on red.
- **Console** — embeds the nuBASIC graphical console window. All `Print` output, graphics
  drawing, and mouse/keyboard input from the running program appear here.

The panel can be resized, hidden, or detached as a floating window via the **Settings** menu.

---

## 8. History and Evolution of nuBASIC

### Origins (March 2014)

nuBASIC was created by Antonino Calderone and first published in March 2014, starting from
version 0.1. The initial implementation established the core of what would become a complete
language: an expression interpreter, variable support, and a set of fundamental math functions
(`Sin`, `Cos`, `ASin`, `ACos`, and the other standard trigonometric and transcendental functions).

Within the same month, the main BASIC statements appeared: `Let`, `Print`, `Input`, `GoTo`,
`If/Then/Else`, `For/To/Step`, and the essential interactive commands `Run`, `List`, `New`,
`Clr`. An I/O console was implemented for both Windows and Linux, making nuBASIC immediately
usable on both platforms. `.deb` packages for Ubuntu/Debian and RPM packages for Fedora and
openSUSE followed shortly after.

### Growing the language (April – June 2014)

Spring 2014 brought a remarkable rate of expansion. Each release added features that
transformed nuBASIC from a toy interpreter into a usable programming tool:

- **File operations** — `Open`, `Close`, `Input#`, `Print#`, `FOpen`, `Seek`, `FTell`, `FSize`,
  and byte arrays (`@` suffix), enabling programs to read and write persistent data (v0.15)
- **Subroutines and functions** — `Sub`/`End Sub`, `Function`/`End Function`, `Exit Sub`,
  `Exit Function`, with proper call stacks and local variable scopes (v0.18, v0.19)
- **Structured loops** — `While`/`Wend` and `Do`/`Loop While`, reducing reliance on `GoTo` (v0.16)
- **Label support** — `GoTo` and `GoSub` could now target named labels as well as line numbers,
  making code far more readable (v0.14)
- **Breakpoints and tracing** — conditional breakpoints (`Break If`), program tracing (`TrOn`/
  `TrOff`), and step-mode execution (`StOn`/`StOff`) for debugging (v0.13–v0.20)
- **First graphics** — `Line`, `Rect`, `FillRect`, `Ellipse`, `FillEllipse` on Windows (GDI)
  and Linux (X11), then `TextOut` and `SetPixel` (v0.7–v0.9)
- **Mouse support** — `GetMouseX()`, `GetMouseY()`, `GetMouseBtn()` (v1.06)
- **`Eval` function** — evaluate a nuBASIC expression from a string at run time (v1.10)
- First game examples: `breakout.bas` (ASCII art), `breakout2.bas` (GDI graphics),
  `minehunter.bas` (the Mine Hunter clone)

In parallel, the nuBasicEditor — a Windows IDE with syntax highlighting, autocomplete,
context help (F1), bookmarks, and a toolbar — grew from its first release (v0.2) to v1.11.

### Type system expansion (Nov 2014 – Nov 2015)

Throughout 2015, nuBASIC received a series of releases that deepened the type system and
brought it closer to modern structured BASIC dialects:

- **Boolean** (`#` postfix) and **Long64** (`&` postfix) numeric types, alongside `True` and
  `False` literal constants (v1.24)
- **Unary operators** — including C-style prefix `++` and `--` (v1.24)
- **Escape sequences** — `\n`, `\r`, `\t`, `\a`, and others inside string literals (v1.24)
- **`Const` statement** — named, immutable constants with optional explicit type (v1.22)
- **`Elif` / `ElseIf`** — cleaner multi-branch conditionals without nesting (v1.25–v1.26)
- **`Stop` statement** — break execution and return to the REPL, resumable with `Cont` (v1.51)
- **Explicit type declarations** — `Dim x As Integer`, `Dim s As String`, eliminating
  ambiguity for complex programs (v1.40)
- **`Struct`** — user-defined composite types with named fields, explicit field types, and
  full nesting support (v1.40)
- **Extended function syntax** — typed return values, functions returning struct objects,
  and functions returning arrays by value (v1.42, v1.48)

New graphical example programs appeared: `clock.bas` (an analog clock), `plane.bas` (a
flying-plane animation), and `GetPixel` was added to allow reading back pixel colors.

### Hash tables, floating-point formatting, and more (Jan 2017)

Version 1.47 added several convenience features that filled gaps in the standard library:

- **Hash tables** — `HSet`, `HGet`, `HCnt`, `HChk`, `HDel` — providing a built-in
  associative container without needing to implement one from scratch
- **`Any` type** — a variable that automatically takes the type of whatever is assigned to it,
  enabling generic containers and the hash table API
- **`StrP` / `StrP$`** — floating-point formatting with a specified number of significant
  digits, filling the gap between `Str$` and manual format strings
- **`GetAppPath()`** — the path of the nuBASIC executable, useful for loading resources
  relative to the install location
- **`Quit()`** — exit the interpreter from within a program

### Linux IDE, MIT license (Jul 2017)

Version 1.48 was a strategic milestone. The IDE was **ported to Linux/GTK+**, making the
full development environment available on both Windows and Linux for the first time. The
license was changed to **MIT**, removing all previous usage restrictions. The release also
added editor zoom in/out, `SetTopmost`, updated stb\_image to 2.15, and Scintilla to 3.74.

### READ / DATA / RESTORE (Jul 2019)

Version 1.50 added the classic `Read`, `Data`, and `Restore` statements. These allow
programs to embed structured constant tables — palettes, maps, configurations, sprite data —
directly in source code, read them sequentially with `Read`, and re-read them from any
position with `Restore`. The feature had been a standard part of BASIC since the 1970s and
its addition closed an important compatibility gap.

### Quality-of-life fixes (Sep 2019)

Version 1.51 addressed several correctness issues:

- Fixed a parser regression where the unary minus operator could trigger a spurious syntax error
- Added `Stop` for compatibility with other BASIC interpreters
- Extended `If` to support an implicit `GoTo` after `Then` and `Else` (e.g.
  `If x Then myLabel` without the `GoTo` keyword)
- Fixed mouse position misalignment on Windows high-DPI / scaled display configurations

### Unicode support (Feb 2022)

Version 1.52 fixed a bug where `Const` did not work correctly with string values. More
significantly, it added support for **Unicode escape sequences** in string literals: by
prefixing a `Print` statement with `$u`, the interpreter recognises `\u0000`–`\uFFFF`
sequences and outputs the corresponding Unicode characters. How they appear depends on the
console font configuration.

### Custom Windows console, UTF-8, MSI installer (March 2026)

Version 1.60 was the largest infrastructure release since the original:

- A **brand-new GDI-based console window** replaced the standard Windows console. The new
  console renders text and graphics natively using GDI, works correctly both standalone and
  embedded in the IDE, avoids the limitations of the Windows Console API (which does not
  support arbitrary pixel graphics), and renders proportional fonts cleanly. The window can
  be moved, resized, and drawn into from BASIC code at any granularity.
- **Full UTF-8 support** end-to-end: characters outside ASCII — Latin Extended, Cyrillic,
  Greek, CJK, emoji — are now preserved through the tokenizer, interpreter runtime, and
  rendered by the GDI console. Two silent-stripping bugs were fixed: one in the CLI
  file-loading path (`nu_interpreter.cc`) and one in the IDE editor's code-rebuild path
  (`nuBasicEditor.cc`). The new example `hello_world_languages.bas` demonstrates multilingual
  `Print` output in 20 languages in a single program.
- An **MSI installer** was introduced, enabling proper Windows Add/Remove Programs
  integration, desktop shortcuts, and clean uninstallation.
- Scintilla was updated to its latest version.

### Struct-returning functions, Screen mode, regression tests (April 2026, v1.62)

Version 1.62 introduced new built-in functions that return all their values in a single struct,
a headless text mode for scripting and CI pipelines, and the first automated regression test suite.

**`GetDateTime()`** replaces the individual scalar date/time accessors with a struct-typed return
value. It returns a `DateTime` struct with fields `year`, `month`, `day`, `hour`, `minute`,
`second`, `wday`, and `yday`. The `DateTime` type is pre-registered at interpreter startup —
`Dim dt As DateTime` works without a user-written `Struct` block. The individual functions
`SysYear`, `SysMonth`, `SysDay`, `SysHour`, `SysMin`, `SysSec`, `SysWDay`, and `SysYDay` are
deprecated and will be removed in nuBASIC v2.0.

**`GetMouse()`** replaces `GetMouseX`, `GetMouseY`, and `GetMouseBtn` with a `Mouse` struct
(`x`, `y`, `btn`). The `Mouse` type is also pre-registered. The individual functions are
deprecated and will be removed in nuBASIC v2.0.

**`SCREEN` statement** — `Screen 0` switches to text/headless mode: I/O goes through the real
console (stdout/stdin) and all GDI drawing calls are silent no-ops. `Screen 1` restores full
GDI console mode. Mirrors the GW-BASIC `SCREEN` command.

**`-t` / `--text-mode` CLI flag** activates `Screen 0` before the interpreter starts and
reconnects the CRT stdio streams to the caller's inherited Win32 handles (pipe, redirect,
terminal), allowing test runners and CI pipelines to capture output without a GUI window. The
interpreter exits after executing a batch command instead of blocking on stdin.

**Regression test suite** — `tests/run_tests.ps1` (PowerShell) and `tests/run_tests.sh` (Bash)
run every `test_*.bas` with `-t -e` and report per-suite and overall pass/fail counts. A
`RunTests` CMake/VS target invokes the appropriate script. Ten test suites are included.

### Flicker-free graphics rendering (April 2026, v1.61)

Version 1.61 addressed the last major visual quality problem: per-primitive screen flicker
in animated programs.

Before this release, every graphics call — `Line`, `FillRect`, `TextOut`, and all others —
triggered an immediate blit of the back buffer to the visible screen. A frame composed of
ten drawing commands produced ten visible intermediate states. Games, clocks, simulations,
and the fractal demos all exhibited this artifact to varying degrees.

Two complementary fixes were implemented at the C++ level:

1. **Painting compositing fix** — Text characters and the text cursor are now composited
   onto the off-screen back buffer *before* the single copy to the screen,
   eliminating the double-paint flicker that occurred during window repaints.

2. **`ScreenLock` / `ScreenUnlock` / `Refresh` instructions** — A new atomic boolean flag
   (`_render_locked`) in `ConsoleWindow` suppresses the automatic `refresh()` call inside
   `release_offscreen_dc()`. The three new BASIC instructions expose this mechanism directly
   to programs. Fourteen graphical example programs shipped with nuBASIC were updated to use
   the pattern: `breakout2.bas`, `breakout3.bas`, `pingpong.bas`, `nutetris.bas`,
   `nutetris3d.bas`, `raycast3d.bas`, `plane.bas`, `clock.bas`, `cartoon.bas`,
   `minesHunter.bas`, `tictactoe.bas`, `line.bas`, `rosetta.bas`, and `calc.bas`.

Built-in help entries for all three new instructions were added to `nu_builtin_help.cc` so
that `Help ScreenLock`, `Help ScreenUnlock`, and `Help Refresh` return full documentation
from the REPL.

## 9. Making nuBASIC: Interpreter Internals

Writing a BASIC interpreter in modern C++ is a surprisingly enjoyable exercise. BASIC itself
carries a strong nostalgia for the early era of personal computing — machines like the
Commodore 64 shipped with a BASIC interpreter in ROM, and for an entire generation that
interpreter *was* the computer. Recreating that experience from scratch, with the tools of
contemporary C++, captures both the simplicity of the original and the correctness that comes
from better type systems and standard-library infrastructure.

This chapter describes the internal architecture of the nuBASIC interpreter. It is aimed at
developers who want to understand how the interpreter works, contribute to the codebase, or
build upon it for their own language experiments. All classes and files mentioned are in the
`nu` namespace; header files are in `include/` and implementations in `lib/`.

---

### 9.1 Main Components

The interpreter follows the classical pipeline of a language processor: source text goes in,
tokens come out, tokens become an abstract syntax tree, and the tree is executed against a
runtime context. The following components implement this pipeline. They do not always map
one-to-one to a single C++ class — some are conceptual groupings — but each has a clear home
in the source tree.

| Component | Class / Type | Key file |
|---|---|---|
| **Tokenizer** | `tokenizer_t` (extends `expr_tknzr_t`) | `include/nu_tokenizer.h` |
| **Token** | `token_t` | `include/nu_token.h` |
| **Token List** | `token_list_t` | `include/nu_token_list.h` |
| **Expression Parser** | `expr_parser_t` | `include/nu_expr_parser.h` |
| **Expression node base** | `expr_any_t` | `include/nu_expr_any.h` |
| **Statement Parser** | `stmt_parser_t` | `include/nu_stmt_parser.h` |
| **Statement base** | `stmt_t` | `include/nu_stmt.h` |
| **Static Program Context** | `prog_ctx_t` | `include/nu_prog_ctx.h` |
| **Runtime Program Context** | `rt_prog_ctx_t` (extends `prog_ctx_t`) | `include/nu_rt_prog_ctx.h` |
| **Program line map** | `prog_line_t` | `include/nu_program.h` |
| **Interpreter** | `interpreter_t` | `include/nu_interpreter.h` |
| **Variant value** | `variant_t` | `include/nu_variant.h` |
| **Built-in help** | `builtin_help_t` | `include/nu_builtin_help.h` |

**Tokenizer** — breaks a source line into a flat sequence of typed `token_t` objects.

**Expression Parser** — reads a token sequence representing an expression such as
`2 + 4 * 17` and returns an `expr_any_t::handle_t` (a `std::shared_ptr<expr_any_t>`) that
can be evaluated on demand by calling its `eval()` method.

**Expression Evaluator** — the `eval()` virtual method on each `expr_any_t` subclass.
Calling it with a `rt_prog_ctx_t` produces a `variant_t` result.

**Statement Parser** (`stmt_parser_t`) — iterates over the token list for a complete source
line and builds an executable statement object tree. It calls the Expression Parser whenever
it encounters an expression.

**Statement Executor** — the `run()` virtual method on each `stmt_t` subclass.

**Static Program Context** (`prog_ctx_t`) — accumulated during the build phase. Holds
procedure prototypes, structure definitions, label tables, and the metadata needed to resolve
multi-line constructs such as `For`/`Next`, `While`/`Wend`, `If`/`ElseIf`/`EndIf`, and
nested procedure definitions.

**Runtime Program Context** (`rt_prog_ctx_t`) — extends `prog_ctx_t` with all the state
that exists only while a program is running: variable values, the procedure call stack,
`For`-loop runtime data, the file-descriptor table, the program counter, and the return-value
registers for functions.

**Interpreter** (`interpreter_t`) — owns the source lines, the parsed program, the static and
runtime contexts, and the statement parser. It implements the CLI command loop, the `rebuild()`
phase, the `run()` loop, and the debugger.

**Built-in Function Library** — individual built-in functions (`Sin`, `Len`, `Mid$`, …) are
implemented as `expr_any_t` subclasses whose `eval()` method performs the computation.

**Language Statement Objects** — each BASIC statement (`Print`, `For`…`Next`, `If`…`EndIf`,
`Sub`, `Open`, …) is a `stmt_t` subclass with a `run(rt_prog_ctx_t&)` override.

**Syntax and Runtime Error Handlers** — C++ exception classes used throughout the pipeline to
report tokenizer, parser, and runtime errors back to the interpreter layer.

**Built-in Help** — the `builtin_help_t` class implements the `Help` and `Apropos` CLI
commands by maintaining a keyword-to-description table compiled into the interpreter.

---

### 9.2 A Line-Oriented Interpreter

BASIC is a line-oriented language: the position of a hard line break in the source is
syntactically significant in a way that it is not in C++ or Python. nuBASIC preserves this
property throughout the entire pipeline.

Source text is stored in two parallel maps inside `interpreter_t`:

- `_source_line` — maps a `line_num_t` to the raw source string, exactly as typed.
- `_prog_line` — a `prog_line_t` object (a `std::map<line_num_t, std::pair<stmt_t::handle_t, dbginfo_t>>`)
  that maps the same line numbers to compiled statement objects plus debug information.

During the build phase (`rebuild()`) each source line is handed to `stmt_parser_t::compile_line()`,
which tokenizes it and calls `parse_block()`. The result — a shared pointer to a block statement
object — is stored in `_prog_line` under that line's number.

The static program context (`prog_ctx_t`) is built at the same time. It stitches together the
multi-line constructs: when the statement parser encounters `For` it records a forward reference
in `for_loop_metadata`; when it later encounters the matching `Next` it resolves that reference.
The same mechanism applies to `While`/`Wend`, `Do`/`Loop While`, `If`/`ElseIf`/`EndIf`,
`Sub`/`Function`/`End Sub`, and `Type`/`End Type`.

At execution time, `run()` iterates over `_prog_line` in key order. For each entry it calls
the `run()` method of the block statement object, passing the `rt_prog_ctx_t`. The runtime
context's program-counter field (`runtime_pc`) is updated after each statement; jump
statements (`Goto`, `Gosub`, `Return`, procedure calls) modify it directly to redirect
execution.

---

### 9.3 Tokens and the Tokenizer

Before any line is parsed, `tokenizer_t` performs lexical analysis: it scans the raw source
string and produces a sequence of `token_t` objects.

Each `token_t` carries:

| Attribute | Member | Description |
|---|---|---|
| Original text | `_org_id` | Preserves the original capitalisation |
| Normalised text | `_identifier` | Lowercase version, used for keyword matching |
| Token class | `_type` (`tkncl_t` enum) | Category of the token |
| Source position | `_position` | Byte offset within the source line |
| Expression pointer | `_expression_ptr` | Shared pointer to the surrounding expression string |

The `tkncl_t` enumeration defines the token categories:

```cpp
enum class tkncl_t {
    UNDEFINED,
    BLANK,
    NEWLINE,
    IDENTIFIER,    // keywords and variable names
    INTEGRAL,      // integer literals
    REAL,          // floating-point literals
    OPERATOR,      // +  -  *  /  ^  =  <>  <  <=  >  >=  \  &
    SUBEXP_BEGIN,  // (
    SUBEXP_END,    // )
    STRING_LITERAL,
    STRING_COMMENT,
    SUBSCR_BEGIN,  // [  (array subscript open)
    SUBSCR_END,    // ]
    LINE_COMMENT   // '  or  Rem
};
```

The Statement Parser uses `_type` to make fast branching decisions — seeing an `IDENTIFIER`
token whose `_identifier` is `"print"` immediately routes to `parse_print()` — without
inspecting the original string again.

The flat enum design (rather than a class hierarchy of token types) was a deliberate choice:
it keeps all `token_t` objects homogeneous and trivially storable in a standard container.

---

### 9.4 Token List Container

The `token_list_t` class wraps a `std::deque<token_t>` and adds convenience operations that
simplify the parser:

- Efficient removal from either end (front-popping as tokens are consumed).
- Marker insertion: `SUBEXP_BEGIN`/`SUBEXP_END` tokens are inserted by the expression parser
  to delimit sub-expressions after operator-precedence reordering.
- Helpers to peek at the front token, check whether the list is empty, and skip blanks.

By encapsulating the deque, `token_list_t` lets the parser code read as a sequence of
`consume-and-test` operations on a stream of tokens rather than as index arithmetic over a
raw array.

---

### 9.5 Parsing the Code

Three distinct parsers handle different levels of the grammar.

**Expression Parser (`expr_parser_t`)** — analyses a single expression such as
`2 + Sin(PI() / 2)` and returns an `expr_any_t::handle_t`: a shared pointer to the root of
an evaluable syntax tree. Its entry points are:

```cpp
expr_any_t::handle_t compile(expr_tknzr_t& tknzr);
expr_any_t::handle_t compile(token_list_t tl, size_t expr_pos);
```

**Statement Parser (`stmt_parser_t`)** — analyses one complete source line and returns a
`stmt_t::handle_t`. Its main entry points are:

```cpp
stmt_t::handle_t compile_line(prog_ctx_t& ctx, const std::string& source_line);
stmt_t::handle_t parse_block(prog_ctx_t& ctx, token_list_t& tl, ...);
stmt_t::handle_t parse_stmt (prog_ctx_t& ctx, token_list_t& tl);
```

`parse_block()` loops while the token list is non-empty, calling `parse_stmt()` on each
iteration. `parse_stmt()` inspects the first token and dispatches to a dedicated
`parse_xxx()` method for each statement keyword: `parse_print()`, `parse_for_to_step()`,
`parse_if()`, `parse_sub()`, and so on.

Whenever a `parse_xxx()` method encounters an expression in the token stream, it calls the
Expression Parser to compile that expression into an `expr_any_t` node. The resulting node is
embedded in the statement object; its `eval()` method will be called at runtime.

**CLI Parser (inside `interpreter_t`)** — handles interpreter-specific commands (`List`,
`Run`, `Load`, `Save`, `New`, `Help`, …). These commands are not BASIC language statements;
they are recognised and dispatched by `interpreter_t::exec_command()`.

---

### 9.6 Expression Parsing in Detail

Expression parsing is where most of the interesting algorithmic work happens. The central
challenge is operator precedence: the expression `2 + 4 * 17` must evaluate to
`2 + (4 * 17) = 70`, not `(2 + 4) * 17 = 102`.

The Expression Parser resolves this in three steps.

**Step 1 — Tokenise.** The expression string is fed to a `tokenizer_t` (or the already-built
`token_list_t` is reused) to produce a flat token sequence.

**Step 2 — Reorder by precedence.** The token list is restructured to encode the operator
precedence order. `SUBEXP_BEGIN`/`SUBEXP_END` marker tokens are inserted to delimit
sub-expressions. Static helpers in `expr_parser_t` — `reduce_brackets()`,
`fix_real_numbers()`, `fix_minus_prefixed_expressions()` — perform normalisation passes.
The precedence order applied is:

1. Unary identity and negation (`+`, `-`)
2. Exponentiation (`^`)
3. Multiplication and floating-point division (`*`, `/`)
4. Integer division (`\`, `Div`)
5. Modulus (`Mod`)
6. Addition and subtraction (`+`, `-`)
7. Comparison (`=`, `<>`, `<`, `<=`, `>`, `>=`)
8. Logical and bitwise (`And`, `Or`, `Xor`, and bit-wise variants)

**Step 3 — Build the syntax tree.** `expr_parser_t::parse()` recursively constructs a tree
of `expr_any_t` nodes. For `2 + 4 * 17` the result is:

```
        binary_expression(+, sum)
              /          \
        literal          binary_expression(*, multiplication)
        (integer, 2)           /        \
                          literal      literal
                          (int, 4)     (int, 17)
```

The `expr_any_t` hierarchy has a node type for each expression element:

- **Binary expression** — one operator, two child `expr_any_t` nodes (left and right).
- **Unary expression** — one operator, one child.
- **Function call** — built-in or user-defined function, with an argument list of
  `expr_any_t` nodes.
- **Variable** — a name looked up in the runtime context at `eval()` time.
- **Literal constant** — an integer, floating-point, or string value embedded at compile time.
- **Empty expression** — a no-op sentinel.

The base class interface is minimal:

```cpp
// include/nu_expr_any.h
class expr_any_t {
public:
    using handle_t    = std::shared_ptr<expr_any_t>;
    using func_args_t = std::vector<expr_any_t::handle_t>;

    virtual variant_t eval(rt_prog_ctx_t& ctx) const = 0;
    virtual bool      empty()  const noexcept = 0;
    virtual std::string name() const noexcept = 0;
    virtual ~expr_any_t() {}
};
```

Every concrete node overrides `eval()` to carry out its specific computation and return a
`variant_t`. The runtime context `rt_prog_ctx_t& ctx` gives access to variable values,
the call stack, and I/O handles.

---

### 9.7 The Variant Type

`variant_t` (`include/nu_variant.h`) is the universal value type of the nuBASIC runtime.
Every expression evaluates to a `variant_t`; every variable stores one.

The supported types are identified by the `variant_t::type_t` enumeration:

| Enumerator | Meaning |
|---|---|
| `UNDEFINED` | Uninitialised / not-set |
| `INTEGER` | 32-bit signed integer |
| `DOUBLE` | Double-precision floating point |
| `STRING` | Text string |
| `BYTEVECTOR` | Raw byte array |
| `BOOLEAN` | Boolean value |
| `STRUCT` | User-defined structure |
| `OBJECT` | Object handle (for GUI/external objects) |
| `ANY` | Wildcard (used in procedure signatures) |

Internally, scalar values are stored in a
`std::vector<std::variant<string_t, integer_t, double_t>>`. The vector has size 1 for
scalars and size *n* for array variables (nuBASIC arrays are implemented as `variant_t` with
`_vect_size > 1`). Structure instances carry their field data in a separate
`std::vector<struct_data_t>`.

Using `variant_t` rather than a C-style `union` or a class hierarchy for each type
dramatically simplifies the evaluator: every arithmetic operation, comparison, string
function, and assignment works with the same type, and implicit type coercions (integer to
double, integer to string, etc.) are handled in one place.

---

### 9.8 Tracing Execution of a Simple Program

The following traces the complete path from keyboard input to printed output for the single-line
program:

```basic
10 Print 2 + 4 * 17
```

**1. Entry point.**
After the program is entered in the REPL, the user types `RUN`. The console read loop obtains
the string `"RUN"` from standard input and calls `interpreter_t::exec_command("RUN")`.

**2. Build phase.**
`exec_command()` recognises `"RUN"` and calls `interpreter_t::rebuild()`.
`rebuild()` clears `_prog_line` and the static context (`prog_ctx_t`), then iterates over
every entry in `_source_line`. For line 10 it calls
`stmt_parser_t::compile_line(ctx, "10 PRINT 2+4*17")`.

`compile_line()` creates a `tokenizer_t` from the source string and calls `parse_block()`.
The tokenizer produces:

```
{ ("print", IDENTIFIER), (" ", BLANK),
  ("2", INTEGRAL), ("+", OPERATOR),
  ("4", INTEGRAL), ("*", OPERATOR), ("17", INTEGRAL) }
```

`parse_block()` calls `parse_stmt()`. The first significant token is `"print"` → dispatches
to `parse_print()`.

**3. Building the Print statement.**
`parse_print()` calls the template helper `parse_arg_list()`. This helper drives the
Expression Parser on the remaining tokens `2 + 4 * 17`. The Expression Parser reorders the
token list to encode precedence and then builds the tree:

```
binary_expression(+)
 ├── literal(integer, 2)
 └── binary_expression(*)
      ├── literal(integer, 4)
      └── literal(integer, 17)
```

The root `expr_any_t::handle_t` is wrapped in an argument list and stored inside a new
`stmt_print_t` object. `parse_print()` returns a `stmt_t::handle_t` pointing to this object.

`parse_block()` wraps it in a block statement handle and returns. `compile_line()` stores the
result in `_prog_line[10]`.

**4. Run phase.**
`rebuild()` finishes and `run()` creates a `program_t` object from `_prog_line` and
`rt_prog_ctx_t`. The program iterates over its line map; for line 10 it calls
`stmt_print_t::run(ctx)`.

**5. Evaluation.**
`stmt_print_t::run()` iterates over its argument list. The single argument is the root
`expr_any_t` node. It calls `node->eval(ctx)`.

`eval()` on the `binary_expression(+)` calls `eval()` on its two children:

- Left child `literal(2)` returns `variant_t(INTEGER, 2)`.
- Right child `binary_expression(*)` evaluates to `variant_t(INTEGER, 68)` (4 × 17).

The `+` node adds them: `variant_t(INTEGER, 70)`.

**6. Output.**
`stmt_print_t::run()` receives `variant_t(INTEGER, 70)`, converts it to the string `"70"`,
and writes it to the output stream. The console displays:

```
70
```

---

### 9.9 Extending the Built-in Function Set

Adding a new built-in function requires touching exactly four source files. The changes are
self-contained: no parser rules need to be modified, no grammar is involved. The interpreter
discovers new functions entirely through the data structures populated in those four files.

The example throughout this section is a 1-D convolution function `Conv`. Given two vectors
*u* = (1, 0, 1) and *v* = (2, 7) — which represent the polynomial coefficients of *x*² + 1
and 2*x* + 7 respectively — `Conv(u, v)` should return the vector (2, 7, 2, 7), i.e. the
coefficients of 2*x*³ + 7*x*² + 2*x* + 7. A nuBASIC program using the new function looks
like:

```basic
Dim u(3) As Double
Dim v(2) As Double
u(0)=1 : u(1)=0 : u(2)=1
v(0)=2 : v(1)=7
Dim w(4) As Double
w = Conv(u, v)
For i = 0 To 3
    Print w(i);" ";
Next i
' output: 2 7 2 7
```

`Conv` will be defined with two overloads:

```
Conv( v1(), v2() )                    ' use full array sizes
Conv( v1(), v2(), count1, count2 )    ' use only the first count1/count2 elements
```

#### The four files to modify

| File | What to add |
|---|---|
| `lib/nu_global_function_tbl.cc` | C++ functor + `fmap["conv"] = conv_functor;` |
| `lib/nu_builtin_help.cc` | One entry in `_help_content[]` |
| `lib/nu_reserved_keywords.cc` | `"conv"` in the `list()` set |

> **Note:** `Conv` is already present in the shipping keyword list and help table — it was
> used as the reference implementation when those extension points were first designed. The
> walkthrough below reconstructs the exact code that lives in those files.

---

#### Step 1 — Implement the C++ algorithm

The pure C++ convolution is independent of the interpreter and easy to unit-test in isolation:

```cpp
template <typename T>
std::vector<T> conv(const std::vector<T>& v1, const std::vector<T>& v2)
{
    const int n = int(v1.size());
    const int m = int(v2.size());
    const int k = n + m - 1;
    std::vector<T> w(k, T());

    for (int i = 0; i < k; ++i) {
        const int jmn = (i >= m - 1) ? i - (m - 1) : 0;
        const int jmx = (i < n - 1)  ? i            : n - 1;
        for (int j = jmn; j <= jmx; ++j)
            w[i] += v1[j] * v2[i - j];
    }
    return w;
}
```

Place this template near the top of `lib/nu_global_function_tbl.cc`, before the
`get_instance()` function.

---

#### Step 2 — Write the functor

Every built-in function is registered as a C++ callable with the signature:

```cpp
variant_t functor_name(
    rt_prog_ctx_t&        ctx,   // runtime execution context
    const std::string&    name,  // function name (one functor can serve several names)
    const nu::func_args_t& args  // vector of unevaluated expression nodes
);
```

`func_args_t` is `std::vector<expr_any_t::handle_t>`. Evaluating `args[i]->eval(ctx)` calls
down into the expression-tree evaluator described in §9.6 and returns a `variant_t`.

The full `conv_functor` implementation:

```cpp
static variant_t conv_functor(
    rt_prog_ctx_t&         ctx,
    const std::string&     name,
    const nu::func_args_t& args)
{
    // 1. Get and validate argument count (2 or 4)
    const auto args_num = args.size();
    rt_error_code_t::get_instance().throw_if(
        args_num != 2 && args_num != 4,
        0, rt_error_code_t::value_t::E_INVALID_ARGS, "");

    // 2. Evaluate the first two arguments (the input vectors)
    //    Left-to-right evaluation order is mandatory — each eval() call may
    //    have side effects on the shared runtime context.
    auto variant_v1 = args[0]->eval(ctx);
    auto variant_v2 = args[1]->eval(ctx);

    const auto actual_v1_size = variant_v1.vector_size();
    const auto actual_v2_size = variant_v2.vector_size();

    // 3. Resolve logical sizes (from optional 3rd/4th arguments, or full array)
    const size_t size_v1 =
        args_num == 4 ? size_t(args[2]->eval(ctx).to_long64()) : actual_v1_size;
    const size_t size_v2 =
        args_num == 4 ? size_t(args[3]->eval(ctx).to_long64()) : actual_v2_size;

    // 4. Validate: logical sizes must be within actual array bounds and >= 1
    rt_error_code_t::get_instance().throw_if(
        size_v1 > actual_v1_size || size_v1 < 1,
        0, rt_error_code_t::value_t::E_INV_VECT_SIZE, args[0]->name());
    rt_error_code_t::get_instance().throw_if(
        size_v2 > actual_v2_size || size_v2 < 1,
        0, rt_error_code_t::value_t::E_INV_VECT_SIZE, args[1]->name());

    // 5. Copy variant contents into plain C++ vectors of double
    std::vector<double> v1(size_v1);
    std::vector<double> v2(size_v2);

    bool ok = variant_v1.copy_vector_content(v1);
    rt_error_code_t::get_instance().throw_if(
        !ok, 0, rt_error_code_t::value_t::E_INV_VECT_SIZE, args[0]->name());

    ok = variant_v2.copy_vector_content(v2);
    rt_error_code_t::get_instance().throw_if(
        !ok, 0, rt_error_code_t::value_t::E_INV_VECT_SIZE, args[1]->name());

    // 6. Compute and return
    auto vr = conv(v1, v2);
    const nu::variant_t result(std::move(vr));
    return result;
}
```

Key points:

- `rt_error_code_t::get_instance().throw_if(condition, line, code, token)` throws a
  structured runtime exception when `condition` is true. The interpreter catches it, formats
  the error message from `lib/nu_error_codes.cc`, and displays it to the user.
  `E_INVALID_ARGS` signals a wrong argument count or type; `E_INV_VECT_SIZE` signals that a
  vector argument has a size inconsistency.
- Arguments are evaluated **in order** (`args[0]`, then `args[1]`, …). Each `eval()` call
  may update variables or advance I/O state, so the order is semantically significant.
- `variant_t::copy_vector_content(std::vector<double>&)` fills the destination vector with
  the double-precision representation of each element and returns `false` if the variant is
  not a vector or the destination is too small.
- The `variant_t(std::move(vr))` constructor (declared in `include/nu_variant.h`) moves the
  result `std::vector<double>` directly into the variant without copying.

---

#### Step 3 — Register the functor

Inside `global_function_tbl_t::get_instance()` in `lib/nu_global_function_tbl.cc`, add one
line to the `fmap` block:

```cpp
fmap["conv"] = conv_functor;
```

`fmap` is the `global_function_tbl_t&` reference returned by `get_instance()`. The key is an
`icstring_t` (case-insensitive string), so nuBASIC users may write `Conv`, `conv`, or `CONV`
interchangeably.

---

#### Step 4 — Add inline help

In `lib/nu_builtin_help.cc`, add an entry to the `_help_content[]` static array:

```cpp
{ lang_item_t::FUNCTION, "conv",
    "Returns a vector of Double as result of convolution of 2 given vectors of numbers",
    "Conv( v1, v2 [, count1, count2 ] )" },
```

The four fields of `help_content_t` are:

| Field | Type | Purpose |
|---|---|---|
| `lang_item` | `lang_item_t` enum | `FUNCTION`, `INSTRUCTION`, `COMMAND`, or `OPERATOR` |
| `lang_id` | `const char*` | Name as it appears in `Help` output (matched case-insensitively) |
| `description` | `const char*` | Free-form description; `\n` produces a new line |
| `signature` | `const char*` | Calling syntax shown under the description |

After this change, the REPL responds to:

```
Help Conv
Apropos convolution
```

---

#### Step 5 — Add to the reserved keyword list

In `lib/nu_reserved_keywords.cc`, add `"conv"` (lower case) to the set returned by
`reserved_keywords_t::list()`:

```cpp
std::set<std::string>& reserved_keywords_t::list()
{
    static std::set<std::string> list = {
        // ...existing keywords...
        "conv",
        // ...
    };
    return list;
}
```

Registering a name here tells the tokenizer to classify it as a keyword identifier
(`tkncl_t::IDENTIFIER`) and makes it available to the auto-completion engine in the IDE.
Built-in function names that are *not* in this list still work at runtime but are not
highlighted in the editor and do not appear in the F12 auto-complete list.

---

#### Summary: four-file checklist

| File | Change |
|---|---|
| `lib/nu_global_function_tbl.cc` | 1. Add the C++ algorithm template. 2. Add the functor. 3. Add `fmap["conv"] = conv_functor;` inside `get_instance()`. |
| `lib/nu_builtin_help.cc` | Add one `help_content_t` entry to `_help_content[]`. |
| `lib/nu_reserved_keywords.cc` | Add `"conv"` to the `list()` set. |

Rebuild the project. The new function is immediately available to any nuBASIC program, returns
a correctly typed `variant_t` vector, raises structured errors on bad input, and is fully
documented in the inline help system.

---

## 10. Building nuBASIC from Source

nuBASIC is written in C++17 and builds with CMake 3.14 or later. The same source tree
produces three distinct artefacts depending on the target platform:

| Artefact | Platform | Description |
|---|---|---|
| `nubasic` / `nubasic.exe` | All | Command-line interpreter (CLI) |
| `NuBasicIDE.exe` + `SciLexer.dll` | Windows | Graphical IDE with integrated debugger |
| `nubasicide` | Linux | GTK+2 graphical IDE |

A *console* build — interpreter only, no graphics, no IDE — is also available for embedded and
minimal environments.

---

### 10.1 Getting the Source

Clone the Git repository:

```
git clone https://github.com/eantcal/nubasic.git
cd nubasic
```

Alternatively, download a release archive from
[github.com/eantcal/nubasic/releases](https://github.com/eantcal/nubasic/releases) and
unpack it.

---

### 10.2 Build Targets Overview

The CMake build graph has the following targets:

| Target | Type | Platforms | Depends on |
|---|---|---|---|
| `nuBasicInterpreter` | static library | all | *(root library)* |
| `nuWinConsole` | static library | Windows | GDI (Windows SDK) |
| `nuBasicCLI` (`nubasic.exe`) | executable | Windows | `nuBasicInterpreter`, `nuWinConsole` |
| `nuBasicIDE` (`NuBasicIDE.exe`) | executable | Windows | `nuBasicInterpreter`, `nuWinConsole`, `SciLexer` |
| `SciLexer` | shared library (`.dll`) | Windows | Scintilla 5.5.3 + Lexilla 5.4.3 |
| `nubasic` | executable | Linux/macOS | `nuBasicInterpreter`, X11 |
| `nubasicide` | executable | Linux | `nuBasicInterpreter`, GTK+2, Scintilla (static) |
| `scintilla` | static library | Linux | GTK+2 |
| `mipjson` / `miptknzr` | static libraries | Linux | *(JSON config for GTK IDE)* |

Scintilla 5.5.3 and Lexilla 5.4.3 are downloaded automatically via CMake `FetchContent` the
first time the build is configured (requires internet access). To use the bundled Scintilla 4
instead, pass `-DSCINTILLA_LOCAL=ON`.

---

### 10.3 Building on Windows

#### Prerequisites

- **Visual Studio 2022** (Community or above) with the *Desktop development with C++*
  workload, or an equivalent MSVC toolchain.
- **CMake 3.14+** — included with Visual Studio 2022, or installable separately.
- **Git** — to clone the repository and for CMake's `FetchContent` to download Scintilla.
- **WiX Toolset 3.x** *(optional)* — required only to build the MSI installer.

#### Configure and build with Visual Studio

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

Select the `nuBasicIDE` or `nuBasicCLI` startup project and press **F5**.

#### Configure and build with Ninja (MSVC)

If you prefer a faster Ninja build from a *Developer Command Prompt*:

```bat
cd nubasic
mkdir build && cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

> **Note:** CMake's root `CMakeLists.txt` includes automatic detection of the MSVC include
> and Windows SDK paths when running under Ninja without `vcvarsall.bat`. If you see
> compiler errors about missing headers, open a *Developer Command Prompt* (which runs
> `vcvarsall.bat` automatically) before invoking CMake.

#### Console-only build on Windows

To build only `nubasic.exe` without the IDE:

```bat
cmake -G "Visual Studio 17 2022" -DWITH_WIN_IDE=OFF ..
cmake --build . --config Release
```

---

### 10.4 Building on Linux

#### Prerequisites — full build (interpreter + GTK IDE)

Install the compiler and required libraries. On Debian/Ubuntu:

```sh
sudo apt-get update
sudo apt-get install build-essential cmake git \
    libx11-dev \
    libgtk2.0-dev libglib2.0-dev libpango1.0-dev \
    libatk1.0-dev libcairo2-dev \
    xterm xmessage
```

> `xterm` and `xmessage` are used at runtime by some nuBASIC built-in functions
> (`Shell`, `MsgBox`). `aplay` is used for `PlaySound`; install it with `alsa-utils` if
> needed.

On Fedora/RHEL/CentOS:

```sh
sudo dnf install gcc-c++ cmake git \
    libX11-devel \
    gtk2-devel glib2-devel pango-devel \
    atk-devel cairo-devel \
    xterm xmessage
```

#### Configure and build

```sh
cd nubasic
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

This produces:

```
/usr/local/bin/nubasic      # console interpreter
/usr/local/bin/nubasicide   # GTK IDE
```

#### Console-only build on Linux

To build only the command-line interpreter without X11 or GTK:

```sh
cmake .. -DWITH_X11=OFF -DWITH_IDE=OFF
make -j$(nproc)
```

This enables the `TINY_NUBASIC_VER` preprocessor flag (console build mode), which strips
all graphics, sound, and IDE components. The resulting `nubasic` binary has no external
dependencies beyond the C++ runtime.

#### IDE only, no console build

If you want the GTK IDE but not the console-only interpreter:

```sh
cmake .. -DWITH_X11=ON -DWITH_IDE=ON
```

Both flags are `ON` by default on Linux, so a plain `cmake ..` already produces both
binaries. Use `-DWITH_IDE=OFF` to suppress `nubasicide` if GTK+2 is not available.

---

### 10.5 Building on macOS

macOS support is at the same level as the Linux console build. GTK+2 and the X11-dependent
features are not available; `WITH_X11` and `WITH_IDE` both default to `OFF` on macOS.

#### Prerequisites

Install Xcode Command Line Tools and CMake:

```sh
xcode-select --install
brew install cmake git
```

#### Configure and build

```sh
cd nubasic
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.logicalcpu)
```

This produces the `nubasic` console interpreter. Install it manually or via:

```sh
sudo make install
```

---

### 10.6 Building on iOS with iSH

[iSH](https://ish.app) is an Alpine Linux shell environment for iOS/iPadOS. It supports the
console build of nuBASIC.

Install iSH from the App Store, then inside its shell:

```sh
apk add g++ make cmake git

git clone https://github.com/eantcal/nubasic.git
cd nubasic
mkdir build && cd build
cmake .. -DWITH_X11=OFF -DWITH_IDE=OFF
make -j2
./nubasic
```

The `-DWITH_X11=OFF -DWITH_IDE=OFF` flags are required because iSH has no display server.

---

### 10.7 CMake Option Reference

All options can be passed on the `cmake` command line as `-D<OPTION>=<VALUE>`.

| Option | Default | Platform | Description |
|---|---|---|---|
| `WITH_X11` | `ON` | Linux | Enable X11 graphics, sound, and xterm/xmessage integration. Set `OFF` for a headless or embedded build. |
| `WITH_IDE` | `ON` | Linux | Build the GTK+2 IDE (`nubasicide`). Requires `WITH_X11=ON` and GTK+2 development libraries. |
| `WITH_WIN_IDE` | `ON` | Windows | Build the Windows IDE (`NuBasicIDE.exe`). Set `OFF` to build only `nubasic.exe`. |
| `SCINTILLA_LOCAL` | `OFF` | Windows/Linux IDE | Use the bundled Scintilla 4.x source under `ide/scintilla/` instead of downloading Scintilla 5.5.3. |
| `SCINTILLA_VERSION` | `"5.5.3"` | Windows/Linux IDE | Scintilla version to download when `SCINTILLA_LOCAL=OFF`. |
| `LEXILLA_VERSION` | `"5.4.3"` | Windows/Linux IDE | Lexilla version to download when `SCINTILLA_LOCAL=OFF`. |
| `NUBASIC_INSTALLER` | `"WIX"` | Windows | Installer generator for CPack: `"WIX"` (MSI, recommended) or `"NSIS"` (legacy setup.exe). |
| `CMAKE_BUILD_TYPE` | `Release` | all | `Release` (optimised, `-O3`) or `Debug` (symbols, `-g`). |

#### Example: debug build of the Windows IDE without downloading Scintilla

```bat
cmake -G "Visual Studio 17 2022" ^
      -DSCINTILLA_LOCAL=ON ^
      ..
cmake --build . --config Debug
```

#### Example: minimal Linux build with no external dependencies

```sh
cmake .. -DWITH_X11=OFF -DWITH_IDE=OFF -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

---

### 10.8 Creating Installers and Packages

#### Windows MSI installer

Requires WiX Toolset 3.x installed and on `PATH`. From the build directory:

```bat
cmake --build . --config Release
cpack -G WIX -C Release
```

This produces a `.msi` file in the build directory. The installer:

- Copies `NuBasicIDE.exe`, `nubasic.exe`, and `SciLexer.dll` to `bin\`.
- Installs example `.bas` files to `examples\`.
- Creates a `nuBASIC` folder in the Start Menu with shortcuts for the IDE, CLI, and
  Uninstall.
- Writes `HKCU\Software\nuBASIC\InstallDir` and `ExamplesDir` registry keys so that the IDE
  can locate the examples at startup.
- Registers the `.bas` file extension and associates it with `NuBasicIDE.exe`.

Alternatively, from inside Visual Studio, build the `CreateInstaller` target (listed under
the *Installer* solution folder).

To use the legacy NSIS installer instead:

```bat
cmake .. -DNUBASIC_INSTALLER=NSIS
cpack -G NSIS -C Release
```

#### Linux DEB package

```sh
cd build
cpack -G DEB
```

This produces a `.deb` package installable with `dpkg -i nubasic-*.deb`.


---

*nuBASIC is open source under the MIT License.*
*Bug reports and feature requests: antonino.calderone@gmail.com*

---
