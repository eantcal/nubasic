#!/usr/local/bin/nubasic
REM The following program is an example of the ALGOL-like BASIC dialect
REM supported by nuBASIC
REM See http://en.wikipedia.org/wiki/Turbo_Basic

INPUT "What is your name?: ", n$

PRINT "Hello "; n$

DO
  s$ = ""

  INPUT "How many stars do you want to print"; s

  FOR i = 1 TO s
    s$ = s$ + "*"
  NEXT i

  PRINT s$

  DO
    INPUT "Do you want to print more stars"; q$
  LOOP WHILE LEN(q$) = 0

  q$ = LCASE$(LEFT$(q$, 1))

LOOP WHILE q$ = "y"

PRINT "Goodbye "; n$

