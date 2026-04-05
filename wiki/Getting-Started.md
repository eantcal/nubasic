# Getting Started

← [Introduction to nuBASIC](Introduction-to-nuBASIC) | Next: [Language Reference](Language-Reference)

---

## Running the Interpreter

```
nubasic                   # interactive REPL
nubasic myprogram.bas     # load and run a file
nubasic -e myprogram.bas  # alternative: exec from command line
```

The interactive REPL (Read-Eval-Print Loop) accepts both single statements and complete programs.
You can type a statement and press Enter to execute it immediately, or enter numbered lines to
build up a program in memory and then type `RUN`.

## Your First Program

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

## Shebang (Linux/macOS)

Make a `.bas` file directly executable on Unix-like systems by adding a shebang line as the
very first line of the file:

```basic
#!/usr/local/bin/nubasic
Print "Hello from a script!"
```

Then mark the file as executable: `chmod +x myscript.bas && ./myscript.bas`

## Built-in Help

The REPL includes a complete built-in help system. You never need to leave the interpreter to
look up a keyword:

```
Help               ' list all keywords grouped by category
Help Print         ' full description and syntax of Print
Help Screen        ' description of the Screen mode command
Help ScreenLock    ' description of the ScreenLock instruction
Help examples      ' list installed example programs with load/run hints
Apropos mouse      ' search all help descriptions for the word "mouse"
```

---

← [Introduction to nuBASIC](Introduction-to-nuBASIC) | Next: [Language Reference](Language-Reference)
