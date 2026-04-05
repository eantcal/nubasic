# Command Reference

← [Graphics and Multimedia](Graphics-and-Multimedia) | Next: [IDE](IDE)

---

## Contents

- [Console Commands](#console-commands)
- [Instructions](#instructions)
- [Built-in Functions](#built-in-functions)
- [Operators Reference](#operators-reference)

---

## Console Commands

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

---

## Instructions

### Program Structure

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `Dim` | `Dim var[(n)] [As Type]` | Declare variable or array |
| `ReDim` | `ReDim var(n)` | Resize array (clears content) |
| `Const` | `Const name [As Type] = value` | Declare a constant |
| `Let` | `[Let] var = expr` | Assign a value (keyword optional) |
| `Sub` | `Sub name([ByRef\|ByVal] params)` … `End Sub` | Define a subroutine |
| `Function` | `Function name([ByRef\|ByVal] params) [As Type]` … `End Function` | Define a function |
| `Call` | `Call name(args)` | Invoke a Sub or Function (optional keyword) |
| `ByRef` | `ByRef param As Type` | Pass parameter by reference (mutations propagate back) |
| `ByVal` | `ByVal param As Type` | Pass parameter by value (default) |
| `Struct` | `Struct name` … `End Struct` | Define a composite type |
| `Include` | `Include "file.bas"` | Load and execute another source file |
| `End` | `End` | Stop program execution |
| `Stop` | `Stop` | Break into REPL (resume with `Cont`) |

### Control Flow

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

### I/O

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

### Timing

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `Delay` / `Sleep` | `Delay seconds` | Pause execution (in seconds) |
| `MDelay` | `MDelay milliseconds` | Pause execution (in milliseconds) |

### Data

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `Data` | `Data expr1[, expr2, …]` | Store constant data values |
| `Read` | `Read var1[, var2, …]` | Read from the Data store |
| `Restore` | `Restore [index]` | Rewind or reposition Data pointer |

### Graphics

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
| `Screen` | `Screen mode` | Set console/graphics mode (0=text, 1=GDI) |
| `ScreenLock` | `ScreenLock` | Suspend screen refresh (begin compositing frame) |
| `ScreenUnlock` | `ScreenUnlock` | Present back buffer to screen (end frame) |
| `Refresh` | `Refresh` | Force immediate blit (lock state unchanged) |

### Miscellaneous

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `ChDir` | `ChDir path$` | Change working directory |
| `Shell` | `Shell "command"` | Execute a shell command |
| `Randomize` | `Randomize` | Seed the random number generator with current time |

---

## Built-in Functions

### Math

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

### String

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

### Keyboard / Input

| Function | Returns | Description |
|----------|---------|-------------|
| `InKey$()` | String | Non-blocking key read; empty string if no key is waiting |
| `Input$(n)` | String | Block until exactly n characters are read |
| `GetVKey()` | Integer | Read next virtual key code from input |

### File

| Function | Returns | Description |
|----------|---------|-------------|
| `EOF(n)` | Integer | 1 if at end-of-file; 0 otherwise |
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

### System / Time

| Function | Returns | Description |
|----------|---------|-------------|
| `SysTime$()` | String | Current local time and date as a string |
| `Time()` | Integer | Seconds elapsed since the Unix Epoch |
| `SysHour()` | Integer | Current hour (0–23) |
| `SysMin()` | Integer | Current minute (0–59) |
| `SysSec()` | Integer | Current second (0–59) |
| `SysDay()` | Integer | Day of the month (1–31) |
| `SysMonth()` | Integer | Month (0–11; January = 0) |
| `SysYear()` | Integer | Full year (e.g. 2026) |
| `SysWDay()` | Integer | Day of week (0=Sunday … 6=Saturday) |
| `SysYDay()` | Integer | Day of year (1–365) |
| `GetPlatId()` | Integer | 1 = Windows, 2 = Linux/other |
| `GetAppPath$()` | String | Full path of the nuBASIC executable |
| `Ver$()` | String | nuBASIC version string |

### Graphics / Window (full build only)

| Function | Returns | Description |
|----------|---------|-------------|
| `Rgb(r,g,b)` | Integer | Compose an RGB color from 0–255 components |
| `GetPixel(x,y)` | Integer | Read the color of a pixel |
| `GetMouseX()` | Integer | Mouse cursor X in pixels |
| `GetMouseY()` | Integer | Mouse cursor Y in pixels |
| `GetMouseBtn()` | Integer | Mouse button bitmask (1=left, 2=middle, 4=right) |
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

### Hash Tables

| Function | Returns | Description |
|----------|---------|-------------|
| `HSet(hash$,key$,val)` | — | Insert or update an entry |
| `HGet(hash$,key$)` | Any | Retrieve the value for a key |
| `HChk(hash$,key$)` | Boolean | True if the key exists in the table |
| `HCnt(hash$)` | Integer | Number of entries in the table |
| `HDel(hash$[,key$])` | — | Delete a single key, or the whole table if key omitted |

### Miscellaneous

| Function | Returns | Description |
|----------|---------|-------------|
| `Not(x)` | Integer | Logical NOT: 1 if x=0, 0 otherwise |
| `bNot(x)` | Integer | Bitwise NOT of x |
| `SizeOf(x)` | Integer | Size in bytes of a variable or entire array |
| `SizeOf@(x)` | Integer | Number of elements in an array |

---

## Operators Reference

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

← [Graphics and Multimedia](Graphics-and-Multimedia) | Next: [IDE](IDE)
