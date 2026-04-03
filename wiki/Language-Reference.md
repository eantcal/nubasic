# Language Reference

← [Getting Started](Getting-Started) | Next: [Graphics and Multimedia](Graphics-and-Multimedia)

---

## Contents

- [Variables and Types](#variables-and-types)
- [Operators](#operators)
- [Control Flow](#control-flow)
- [Subroutines and Functions](#subroutines-and-functions)
- [Structures](#structures)
- [Arrays](#arrays)
- [Hash Tables](#hash-tables)
- [File I/O](#file-io)
- [DATA, READ, RESTORE](#data-read-restore)
- [String Handling](#string-handling)

---

## Variables and Types

Variables in nuBASIC can be declared explicitly with `Dim` or created implicitly on first
assignment. The **type suffix** appended to the variable name determines its storage type.

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

Explicit declaration with optional type annotation:

```basic
Dim counter As Integer
Dim name    As String
Dim ratio   As Double
Dim items(99) As Integer   ' array of 100 integers (indices 0..99)
Dim flag    As Boolean
```

Constants are declared with `Const` and cannot be reassigned:

```basic
Const MAX_SIZE As Integer = 100
Const APP_NAME$ = "nuBASIC Demo"
Const PI_APPROX = 3.14159265
```

---

## Operators

### Arithmetic

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

Increment and decrement prefixes:

```basic
++counter%     ' equivalent to: counter% = counter% + 1
--counter%     ' equivalent to: counter% = counter% - 1
```

### Comparison

```basic
If a = b  Then ...   ' equal to
If a <> b Then ...   ' not equal to
If a < b  Then ...   ' less than
If a > b  Then ...   ' greater than
If a <= b Then ...   ' less than or equal to
If a >= b Then ...   ' greater than or equal to
```

### Logical

```basic
If x > 0 And y > 0  Then Print "both positive"
If x = 0 Or  y = 0  Then Print "at least one zero"
If Not(flag#)        Then Print "flag is false"
result# = (a > b) Xor (c > d)
```

### Bitwise

```basic
result = a bAnd b    ' bitwise AND
result = a bOr  b    ' bitwise OR
result = a bXor b    ' bitwise XOR
result = bNot(a)     ' bitwise NOT
result = a bShl 2    ' shift left 2 positions
result = a bShr 2    ' shift right 2 positions
```

Hexadecimal literals use the `&h` prefix:

```basic
mask%   = &hFF000000
red%    = &h0000FF
white%  = &hFFFFFF
```

---

## Control Flow

### If / ElIf / Else

Single-line form:

```basic
If score% > 100 Then Print "High score!" Else Print "Keep trying"
```

Multi-line form:

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

`ElseIf` is accepted as an alias for `ElIf`.

### For / Next

```basic
' Count from 1 to 10
For i% = 1 To 10
   Print i%
Next i%

' Custom step
For x = 0.0 To 1.0 Step 0.25
   Print x
Next x

' Count down
For i% = 10 To 1 Step -1
   Print i%
Next i%

' Exit early
For i% = 1 To 1000
   If i% Mod 7 = 0 And i% Mod 11 = 0 Then
      Print "First multiple of both 7 and 11: "; i%
      Exit For
   End If
Next i%
```

### While / Wend

```basic
While Not(EOF(filenum%))
   Input# filenum%, line$
   Print line$
Wend

While 1
   key$ = InKey$()
   If key$ = "q" Or key$ = "Q" Then Exit While
   MDelay 10
Wend
```

### Do / Loop While

```basic
' Wait for any key press — body runs at least once
Do
   key$ = InKey$()
Loop While Len(key$) = 0

' Retry up to 10 times
Do
   ++attempts%
   result% = TryOperation()
   If result% = 0 Then Exit Do
Loop While attempts% < 10
```

### GoTo / GoSub / Return

```basic
GoTo gameLoop

init:
   score% = 0
   lives% = 3

gameLoop:
   If lives% = 0 Then GoTo gameOver
   GoTo gameLoop

gameOver:
   Print "Game over! Score: "; score%
   End
```

`GoSub` calls a subroutine; `Return` jumps back to the statement after the `GoSub` call:

```basic
GoSub DrawScreen
GoTo mainLoop

DrawScreen:
   Cls
   FillRect 0, 0, 640, 480, &h000000
   Return
```

### On / GoTo (Computed Branch)

```basic
On game_mode% GoTo beginner, intermediate, expert

beginner:
   mines% = 10 : GoTo setup_done
intermediate:
   mines% = 20 : GoTo setup_done
expert:
   mines% = 40
setup_done:
```

---

## Subroutines and Functions

### Sub — No Return Value

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

ClearArea 0, 0, 640, 480
PrintCentered "Welcome to nuBASIC", 12, &hffffff
```

### Function — Returns a Value

```basic
Function Factorial%(n%)
   If n% <= 1 Then
      Factorial% = 1
   Else
      Factorial% = n% * Factorial%(n% - 1)
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

Print Factorial%(12)
Print Clamp(1.5, 0.0, 1.0)
Print Greeting$("World")
```

### ByRef and ByVal

Parameters are **by value** (`ByVal`) by default — the procedure receives a copy and changes
do not affect the caller. Prefix a parameter with `ByRef` to pass by reference: mutations
inside the procedure propagate back to the caller's variable.

```basic
Sub Swap(ByRef a% As Integer, ByRef b% As Integer)
   Dim tmp% As Integer
   tmp% = a% : a% = b% : b% = tmp%
End Sub

Dim x% As Integer, y% As Integer
x% = 7 : y% = 42
Call Swap(x%, y%)
Print x%, y%   ' 42   7
```

Struct variables can also be passed `ByRef`:

```basic
Sub Translate(ByRef p As Point, dx As Double, dy As Double)
   p.x = p.x + dx
   p.y = p.y + dy
End Sub
```

### Call keyword

`Call` is an optional keyword before any Sub/Function invocation. When used, arguments must
be enclosed in parentheses:

```basic
Call ClearArea(0, 0, 640, 480)   ' same as: ClearArea 0, 0, 640, 480
```

### Include directive

`Include "filename.bas"` (also `#Include`) loads and executes another source file at the
point of the directive — useful for splitting programs across multiple files:

```basic
Include "utils.bas"
Call DrawBorder(0, 0, 639, 479)
```

---

## Structures

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

Arrays of structures:

```basic
Dim enemies(10) As Sprite
enemies(0).name$ = "Goblin"
enemies(0).pos.x = 50
```

---

## Arrays

```basic
Dim scores%(9)      ' 10 integers, indices 0..9
Dim names$(4)       ' 5 strings, indices 0..4
Dim data(99)        ' 100 doubles, indices 0..99
```

Access and assignment:

```basic
scores%(0) = 95
scores%(1) = 87

total% = 0
For i% = 0 To 9
   total% = total% + scores%(i%)
Next i%
Print "Average: "; total% / 10
```

`ReDim` resizes an existing array (clears content):

```basic
size% = 100
ReDim scores%(size% - 1)
```

Byte arrays for binary data:

```basic
Dim buf@(1023)
buf@(0) = 65
Print Chr$(buf@(0))   ' A
```

Multi-dimensional data (manual row-major indexing):

```basic
Const W% = 10
Dim grid%(W% * W% - 1)
grid%(2 * W% + 3) = 42   ' element at row=2, col=3
```

---

## Hash Tables

```basic
' Insert or update entries
HSet "config", "width",     800
HSet "config", "height",    600
HSet "config", "title$",    "My Game"
HSet "config", "fullscreen#", False

' Retrieve values
w% = HGet%("config", "width")
t$ = HGet$("config", "title$")
Print t$; " — "; w%

' Check whether a key exists
If HChk("config", "fullscreen#") Then
   fs# = HGet#("config", "fullscreen#")
End If

' Count entries
Print "Config entries: "; HCnt("config")

' Delete a single key
HDel "config", "fullscreen#"

' Delete the entire table
HDel "config"
```

---

## File I/O

### Sequential Text Files

```basic
' Write
Open "notes.txt" For Output As #1
Print# 1, "First line"
Print# 1, "Second line"
Close #1

' Read line by line
Open "notes.txt" For Input As #2
While Not(EOF(2))
   Input# 2, line$
   Print line$
Wend
Close #2

' Append
Open "notes.txt" For Append As #1
Print# 1, "New entry at " + SysTime$()
Close #1
```

### Binary and Random Access

```basic
' Write raw bytes
FOpen "data.bin", "wb", 1
Dim buf@(3)
buf@(0) = &hDE : buf@(1) = &hAD : buf@(2) = &hBE : buf@(3) = &hEF
Print# 1, buf@
Close #1

' Seek and read
FOpen "data.bin", "rb", 2
Seek 2, 2, 2
Read# 2, b@, 1
Print "Byte at offset 2: "; Hex$(b@)
Close #2
```

Diagnostic functions:

```basic
pos%  = FTell(1)    ' current byte offset
size% = FSize(1)    ' total file size in bytes
err%  = FError(1)   ' non-zero if error occurred
at_end% = EOF(1)    ' 1 if at end of file
```

### File System Operations

```basic
MkDir("saves")
RmDir("saves/old")
Erase("temp.txt")
Print Pwd$()
ChDir ".."
```

---

## DATA, READ, RESTORE

```basic
' Embed a color palette
Data "red",    &hFF0000
Data "green",  &h00FF00
Data "blue",   &h0000FF
Data "yellow", &hFFFF00
Data "white",  &hFFFFFF

' Read sequentially
For i% = 0 To 4
   Read name$, color%
   FillRect i%*60, 10, i%*60+50, 60, color%
   TextOut  i%*60+5, 70, name$, &hFFFFFF
Next i%

' Rewind to start
Restore
Read first_name$, first_color%

' Jump to specific position (0-based)
Restore 2
Read name$, color%

' Clear the data store
Restore -1
```

---

## String Handling

### Measuring and Slicing

```basic
s$ = "Hello, World!"

Print Len(s$)           ' 13
Print Left$(s$, 5)      ' "Hello"
Print Right$(s$, 6)     ' "World!"
Print Mid$(s$, 8, 5)    ' "World"   (1-based)
Print SubStr$(s$, 7, 5) ' "World"   (0-based)
```

### Searching

```basic
Print InStr("Hello World", "world")     ' 7  (case-insensitive)
Print InStr("Hello World", "xyz")       ' -1 (not found)
Print InStrCS("Hello World", "World")   ' 7  (case-sensitive)
Print InStrCS("Hello World", "world")   ' -1
```

### Modifying and Building

```basic
board$ = ".........."
board$ = PStr$(board$, 3, "*")   ' "..*......."
board$ = PStr$(board$, 7, "F")   ' "..*.....F."
```

### Case and Character Conversion

```basic
Print UCase$("hello")    ' "HELLO"
Print LCase$("WORLD")    ' "world"
Print Asc("A")           ' 65
Print Chr$(65)           ' "A"
Print Spc(4)             ' "    "
```

### Numeric Conversions

```basic
n% = Val%("42")
x  = Val("3.14")
Print Str$(3.14)          ' "3.14"
Print Hex$(255)           ' "FF"
Print StrP$(3.14159, 4)   ' "3.142"
```

### Escape Sequences and Unicode

Standard escape sequences:

```basic
Print "Column1\tColumn2\tColumn3"
Print "Line 1\nLine 2"
```

Unicode characters with `$u` prefix:

```basic
Print $u, "Caf\u00e9 — \u00e0 \u00e8 \u00ec"   ' Café — à è ì
Print $u, "\u03b1 \u03b2 \u03b3"               ' α β γ  (Greek)
Print $u, "\u4e2d\u6587"                        ' 中文
```

### Dynamic Evaluation

```basic
x = 5
expr$ = "x * x + 2 * x + 1"
Print Eval(expr$)       ' 36
```

---

← [Getting Started](Getting-Started) | Next: [Graphics and Multimedia](Graphics-and-Multimedia)
