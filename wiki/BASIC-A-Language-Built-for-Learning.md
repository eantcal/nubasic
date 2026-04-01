# BASIC: A Language Built for Learning

← [Home](Home) | Next: [Introduction to nuBASIC](Introduction-to-nuBASIC)

---

## Origins

BASIC — **B**eginner's **A**ll-purpose **S**ymbolic **I**nstruction **C**ode — was designed in 1964
at Dartmouth College by John G. Kemeny and Thomas E. Kurtz. Their goal was simple and radical for
its time: give students with no prior programming experience a language they could learn and use
productively in a single sitting. Every design decision was made with that beginner in mind.

Unlike Fortran or COBOL, which required separate compilation steps and knowledge of job-control
languages, BASIC ran interactively. You typed a line, you pressed Enter, you got a result. You
numbered your lines, typed `RUN`, and the program executed immediately. There was no barrier
between thought and execution.

## The Era of Home Computing

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

## Dialects and Evolution

BASIC was never a single, standardised language. Each hardware vendor wrote their own interpreter,
and dialects multiplied. Microsoft's BASIC (later GW-BASIC and QuickBASIC) became the most
widespread on IBM-compatible PCs. Turbo Basic, PowerBASIC, and eventually Visual Basic followed,
each adding structured programming features, better type systems, and graphical capabilities.

The structured revolution in BASIC arrived in the mid-1980s: `WHILE`/`WEND` replaced `GOTO`-heavy
loops, `SUB` and `FUNCTION` replaced line-numbered subroutines, and compilers made BASIC programs
run at near-native speed. Modern successors such as QB64, FreeBASIC, and PureBasic carry that
tradition forward today.

## What All BASIC Dialects Share

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

← [Home](Home) | Next: [Introduction to nuBASIC](Introduction-to-nuBASIC)
