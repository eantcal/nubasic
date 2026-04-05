' test_math.bas
' Tests:
'   1.  Basic arithmetic (integer)
'   2.  Basic arithmetic (double)
'   3.  Integer division (Div) and Modulo (Mod)  — infix operators
'   4.  Abs
'   5.  Int (floor)
'   6.  Sqr (square root)
'   7.  Pow (exponentiation)
'   8.  Min / Max
'   9.  Sign
'  10.  Pi constant
'  11.  Sin / Cos identity  (sin^2 + cos^2 = 1)
'  12.  Bitwise operators (Band, Bor, Bxor, Bshl, Bshr) — infix
'  13.  Logical Not (function, returns 0/1)

Dim pass% As Integer
Dim fail% As Integer
pass% = 0
fail% = 0

Sub AssertEq(label$, got$, expected$)
    If got$ = expected$ Then
        Print "  PASS  " + label$
        pass% = pass% + 1
    Else
        Print "  FAIL  " + label$ + "  got='" + got$ + "'  expected='" + expected$ + "'"
        fail% = fail% + 1
    End If
End Sub

Sub AssertFuzzy(label$, a As Double, b As Double)
    ' Use relative tolerance to handle Pi() returning 7 significant digits
    Dim diff As Double
    diff = Abs(a - b)
    Dim tol As Double
    tol = Abs(b) * 0.000001
    If tol < 0.000001 Then tol = 0.000001
    If diff < tol Then
        Print "  PASS  " + label$
        pass% = pass% + 1
    Else
        Print "  FAIL  " + label$ + "  got=" + Str$(a) + "  expected=" + Str$(b)
        fail% = fail% + 1
    End If
End Sub

' -----------------------------------------------------------------------
' 1. Basic arithmetic (integer)
' -----------------------------------------------------------------------
Print "--- 1. Integer arithmetic ---"

AssertEq "3 + 4",     Str$(3 + 4),     "7"
AssertEq "10 - 3",    Str$(10 - 3),    "7"
AssertEq "6 * 7",     Str$(6 * 7),     "42"
AssertEq "20 / 4",    Str$(20 / 4),    "5"
AssertEq "2 + 3 * 4", Str$(2 + 3 * 4), "14"
AssertEq "(2+3)*4",   Str$((2+3)*4),   "20"

' -----------------------------------------------------------------------
' 2. Basic arithmetic (double)
' -----------------------------------------------------------------------
Print "--- 2. Double arithmetic ---"

AssertFuzzy "1.5 + 2.5", 1.5 + 2.5, 4.0
AssertFuzzy "5.0 / 2.0", 5.0 / 2.0, 2.5
AssertFuzzy "0.1 + 0.2", 0.1 + 0.2, 0.3
AssertFuzzy "3.0 * 1.5", 3.0 * 1.5, 4.5

' -----------------------------------------------------------------------
' 3. Div (integer division) and Mod — INFIX operators: a Div b
' -----------------------------------------------------------------------
Print "--- 3. Div / Mod (infix) ---"

AssertEq "17 Div 5", Str$(17 Div 5), "3"
AssertEq "17 Mod 5", Str$(17 Mod 5), "2"
AssertEq "10 Div 2", Str$(10 Div 2), "5"
AssertEq "10 Mod 2", Str$(10 Mod 2), "0"
AssertEq "-7 Mod 3", Str$(-7 Mod 3), "-1"

' -----------------------------------------------------------------------
' 4. Abs
' -----------------------------------------------------------------------
Print "--- 4. Abs ---"

AssertEq "abs(5)",   Str$(Abs(5)),  "5"
AssertEq "abs(-5)",  Str$(Abs(-5)), "5"
AssertEq "abs(0)",   Str$(Abs(0)),  "0"
AssertFuzzy "abs(-3.14)", Abs(-3.14), 3.14

' -----------------------------------------------------------------------
' 5. Int (floor)
' -----------------------------------------------------------------------
Print "--- 5. Int ---"

AssertEq "int(3.9)",  Str$(Int(3.9)),  "3"
AssertEq "int(-3.1)", Str$(Int(-3.1)), "-4"
AssertEq "int(0.0)",  Str$(Int(0.0)),  "0"

' -----------------------------------------------------------------------
' 6. Sqr
' -----------------------------------------------------------------------
Print "--- 6. Sqr ---"

AssertFuzzy "sqr(4)",   Sqr(4.0), 2.0
AssertFuzzy "sqr(9)",   Sqr(9.0), 3.0
AssertFuzzy "sqr(2)",   Sqr(2.0), 1.4142135623730951
AssertFuzzy "sqr(0)",   Sqr(0.0), 0.0

' -----------------------------------------------------------------------
' 7. Pow
' -----------------------------------------------------------------------
Print "--- 7. Pow ---"

AssertFuzzy "pow(2,10)", Pow(2.0, 10.0), 1024.0
AssertFuzzy "pow(3,3)",  Pow(3.0, 3.0),  27.0
AssertFuzzy "pow(5,0)",  Pow(5.0, 0.0),  1.0
AssertFuzzy "pow(4,0.5)",Pow(4.0, 0.5),  2.0

' -----------------------------------------------------------------------
' 8. Min / Max
' -----------------------------------------------------------------------
Print "--- 8. Min/Max ---"

AssertEq "min(3,7)",  Str$(Min(3, 7)),  "3"
AssertEq "min(7,3)",  Str$(Min(7, 3)),  "3"
AssertEq "min(-1,0)", Str$(Min(-1, 0)), "-1"
AssertEq "max(3,7)",  Str$(Max(3, 7)),  "7"
AssertEq "max(7,3)",  Str$(Max(7, 3)),  "7"
AssertEq "max(-1,0)", Str$(Max(-1, 0)), "0"

' -----------------------------------------------------------------------
' 9. Sign
' -----------------------------------------------------------------------
Print "--- 9. Sign ---"

AssertEq "sign(10)",  Str$(Sign(10)),  "1"
AssertEq "sign(-10)", Str$(Sign(-10)), "-1"
AssertEq "sign(0)",   Str$(Sign(0)),   "0"

' -----------------------------------------------------------------------
' 10. Pi
' -----------------------------------------------------------------------
Print "--- 10. Pi ---"

AssertFuzzy "Pi()",   Pi(),         3.141592653589793
AssertFuzzy "2*Pi()", 2.0 * Pi(),   6.283185307179586

' -----------------------------------------------------------------------
' 11. Sin/Cos identity: sin^2 + cos^2 = 1
' NOTE: nuBASIC parses Sin(x)*Sin(x) incorrectly as Sin(x*Sin(x)).
'       Use intermediate variables to avoid this parser issue.
' -----------------------------------------------------------------------
Print "--- 11. Sin/Cos identity ---"

Dim angle As Double
Dim s As Double
Dim c As Double

angle = Pi() / 4.0
s = Sin(angle)
c = Cos(angle)
AssertFuzzy "sin^2+cos^2 (pi/4)", s*s + c*c, 1.0

angle = Pi() / 3.0
s = Sin(angle)
c = Cos(angle)
AssertFuzzy "sin^2+cos^2 (pi/3)", s*s + c*c, 1.0

s = Sin(0.0)
c = Cos(0.0)
AssertFuzzy "sin(0)", s, 0.0
AssertFuzzy "cos(0)", c, 1.0

' -----------------------------------------------------------------------
' 12. Bitwise operators — INFIX: a Band b, a Bor b, etc.
' -----------------------------------------------------------------------
Print "--- 12. Bitwise (infix) ---"

AssertEq "12 Band 10", Str$(12 Band 10), "8"
AssertEq "12 Bor 10",  Str$(12 Bor 10),  "14"
AssertEq "12 Bxor 10", Str$(12 Bxor 10), "6"
AssertEq "1 Bshl 4",   Str$(1 Bshl 4),   "16"
AssertEq "16 Bshr 4",  Str$(16 Bshr 4),  "1"

' -----------------------------------------------------------------------
' 13. Logical Not (function, returns 0 or 1)
' -----------------------------------------------------------------------
Print "--- 13. Not ---"

AssertEq "not(0)",  Str$(Not(0)),  "1"
AssertEq "not(1)",  Str$(Not(1)),  "0"
AssertEq "not(-1)", Str$(Not(-1)), "0"

' -----------------------------------------------------------------------
' Summary
' -----------------------------------------------------------------------
Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
