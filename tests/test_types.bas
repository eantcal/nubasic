' test_types.bas
' Tests:
'   1. Integer type
'   2. Double type
'   3. String type
'   4. Boolean expressions (comparison operators return 0/1)
'   5. Const declarations
'   6. Type coercion: Val / Str$ / Int
'   7. Mixed-type expressions
'
' Notes on nuBASIC boolean semantics:
'   - Comparison operators (>, <, =, <>, >=, <=) return  1 (true) or 0 (false)
'   - Not(x) returns 1 if x=0, else 0
'   - And / Or are INFIX operators returning "true"/"false" (string)
'   - For numeric boolean use: If x > 0 Then ... style tests

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

' -----------------------------------------------------------------------
' 1. Integer type
' -----------------------------------------------------------------------
Print "--- 1. Integer ---"

Dim i% As Integer
i% = 42
AssertEq "integer assign",  Str$(i%),      "42"
AssertEq "integer negative", Str$(-i%),    "-42"
AssertEq "integer add",      Str$(i% + 8), "50"
AssertEq "integer sub",      Str$(i% - 2), "40"
AssertEq "integer mul",      Str$(i% * 2), "84"
AssertEq "integer zero",     Str$(i% - 42),"0"

' -----------------------------------------------------------------------
' 2. Double type
' -----------------------------------------------------------------------
Print "--- 2. Double ---"

Dim d As Double
d = 3.14159
' Str$ always shows 6 decimal places; compare integer part scaled by 100000
AssertEq "double 5 decimals (scaled)", Str$(Int(d * 100000)), "314159"

Dim f As Double
f = 1.0 / 3.0
' 1/3 > 0 should be true (1)
AssertEq "1/3 > 0 is true", Str$(f > 0.0), "1"

' -----------------------------------------------------------------------
' 3. String type
' -----------------------------------------------------------------------
Print "--- 3. String ---"

Dim s$ As String
s$ = "nuBASIC"
AssertEq "string assign",  s$,             "nuBASIC"
AssertEq "string len",     Str$(Len(s$)),  "7"
AssertEq "string concat",  s$ + " rocks", "nuBASIC rocks"
AssertEq "string empty",   "" + s$,        "nuBASIC"

Dim t$ As String
t$ = ""
AssertEq "empty string len", Str$(Len(t$)), "0"

' -----------------------------------------------------------------------
' 4. Boolean expressions
' Comparison operators return 1 (true) or 0 (false)
' Not(x) returns 1 if x=0, else 0
' -----------------------------------------------------------------------
Print "--- 4. Boolean ---"

Dim b As Integer
b = (5 > 3)
AssertEq "5>3 -> 1",   Str$(b), "1"

b = (3 > 5)
AssertEq "3>5 -> 0",   Str$(b), "0"

b = (5 = 5)
AssertEq "5=5 -> 1",   Str$(b), "1"

b = (5 <> 5)
AssertEq "5<>5 -> 0",  Str$(b), "0"

b = (3 >= 3)
AssertEq "3>=3 -> 1",  Str$(b), "1"

b = (3 <= 2)
AssertEq "3<=2 -> 0",  Str$(b), "0"

' Not function (returns 0 or 1)
AssertEq "not(0) -> 1",  Str$(Not(0)),  "1"
AssertEq "not(1) -> 0",  Str$(Not(1)),  "0"
AssertEq "not(-1) -> 0", Str$(Not(-1)), "0"

' -----------------------------------------------------------------------
' 5. Const declarations
' -----------------------------------------------------------------------
Print "--- 5. Const ---"

Const MAX_VAL% = 100
Const PI_APPROX = 3.14159
Const GREETING$ = "Hi"

AssertEq "const integer",  Str$(MAX_VAL%),               "100"
AssertEq "const double (scaled)", Str$(Int(PI_APPROX * 100)), "314"
AssertEq "const string",   GREETING$,                    "Hi"
AssertEq "const in expr",  Str$(MAX_VAL% * 2),           "200"

' -----------------------------------------------------------------------
' 6. Type coercion: Val / Str$ / Int
' -----------------------------------------------------------------------
Print "--- 6. Type coercion ---"

AssertEq "str$(int)",     Str$(123),       "123"
AssertEq "val(int str)",  Str$(Val("42")), "42"
AssertEq "int(3.9)",      Str$(Int(7.9)),  "7"
AssertEq "int(negative)", Str$(Int(-2.3)), "-3"

' Val("3.14") is a float; compare numerically
AssertEq "val('3.14') > 3.13", Str$(Val("3.14") > 3.13), "1"

' -----------------------------------------------------------------------
' 7. Mixed-type expressions
' -----------------------------------------------------------------------
Print "--- 7. Mixed expressions ---"

Dim n% As Integer
Dim x As Double
n% = 3
x = 1.5
AssertFuzzy3 "int + double = 4.5", n% + x, 4.5
AssertFuzzy3 "int * double = 4.5", n% * x, 4.5

' Mixed integer+double arithmetic (result is double)
' Note: comparing Integer variable to Double literal returns "true"/"false" string,
'       not 0/1 — use same-type comparisons for reliable results.
AssertFuzzy3 "3 + 1.5 = 4.5", n% + 1.5, 4.5

GoTo EndHelpers

Sub AssertFuzzy3(label$, a As Double, b As Double)
    If Abs(a - b) < 0.000001 Then
        Print "  PASS  " + label$
        pass% = pass% + 1
    Else
        Print "  FAIL  " + label$ + "  got=" + Str$(a) + "  expected=" + Str$(b)
        fail% = fail% + 1
    End If
End Sub

EndHelpers:

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
