' test_procedures.bas
' Tests:
'   1. Basic SUB with no parameters
'   2. SUB with value parameters
'   3. FUNCTION returning Integer
'   4. FUNCTION returning String
'   5. FUNCTION returning Double
'   6. Recursive FUNCTION (factorial)
'   7. Recursive FUNCTION (Fibonacci)
'   8. ByRef scalar parameters (SUB)
'   9. ByRef string parameter (SUB)
'  10. Mutual calls: function calling another function

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
' 1. Basic SUB with no parameters (uses global variable)
' -----------------------------------------------------------------------
Print "--- 1. Basic SUB ---"

Dim g_counter% As Integer
g_counter% = 0

Sub Increment()
    g_counter% = g_counter% + 1
End Sub

Call Increment()
Call Increment()
Call Increment()
AssertEq "counter after 3 calls", Str$(g_counter%), "3"

' -----------------------------------------------------------------------
' 2. SUB with value parameters (no side effect on caller)
' -----------------------------------------------------------------------
Print "--- 2. SUB value params ---"

Dim accum% As Integer
accum% = 0

Sub AddTo(val% As Integer)
    accum% = accum% + val%
End Sub

Call AddTo(10)
Call AddTo(20)
Call AddTo(5)
AssertEq "accum after adds", Str$(accum%), "35"

' -----------------------------------------------------------------------
' 3. FUNCTION returning Integer
' -----------------------------------------------------------------------
Print "--- 3. FUNCTION -> Integer ---"

Function Square%(n% As Integer)
    Square% = n% * n%
End Function

AssertEq "square(5)",  Str$(Square%(5)),  "25"
AssertEq "square(0)",  Str$(Square%(0)),  "0"
AssertEq "square(-3)", Str$(Square%(-3)), "9"

' -----------------------------------------------------------------------
' 4. FUNCTION returning String
' -----------------------------------------------------------------------
Print "--- 4. FUNCTION -> String ---"

Function Repeat$(s$ As String, n% As Integer)
    Dim res$ As String
    Dim i% As Integer
    res$ = ""
    ' Guard needed: FOR..NEXT in nuBASIC always executes at least once
    If n% > 0 Then
        For i% = 1 To n%
            res$ = res$ + s$
        Next i%
    End If
    Repeat$ = res$
End Function

AssertEq "repeat 'ab' 3x", Repeat$("ab", 3), "ababab"
AssertEq "repeat 'x' 0x",  Repeat$("x", 0),  ""

' -----------------------------------------------------------------------
' 5. FUNCTION returning Double
' -----------------------------------------------------------------------
Print "--- 5. FUNCTION -> Double ---"

Function Average(a As Double, b As Double)
    Average = (a + b) / 2.0
End Function

AssertEq "avg(3,7)", Str$(Average(3.0, 7.0)), "5"
AssertEq "avg(1,1)", Str$(Average(1.0, 1.0)), "1"

' -----------------------------------------------------------------------
' 6. Recursive FUNCTION: factorial
' -----------------------------------------------------------------------
Print "--- 6. Recursive factorial ---"

Function Fact%(n% As Integer)
    If n% <= 1 Then
        Fact% = 1
    Else
        Fact% = n% * Fact%(n% - 1)
    End If
End Function

AssertEq "fact(0)", Str$(Fact%(0)), "1"
AssertEq "fact(1)", Str$(Fact%(1)), "1"
AssertEq "fact(5)", Str$(Fact%(5)), "120"
AssertEq "fact(7)", Str$(Fact%(7)), "5040"

' -----------------------------------------------------------------------
' 7. Recursive FUNCTION: Fibonacci
' -----------------------------------------------------------------------
Print "--- 7. Recursive Fibonacci ---"

Function Fib%(n% As Integer)
    If n% <= 1 Then
        Fib% = n%
    Else
        Fib% = Fib%(n% - 1) + Fib%(n% - 2)
    End If
End Function

AssertEq "fib(0)", Str$(Fib%(0)), "0"
AssertEq "fib(1)", Str$(Fib%(1)), "1"
AssertEq "fib(7)", Str$(Fib%(7)), "13"
AssertEq "fib(10)", Str$(Fib%(10)), "55"

' -----------------------------------------------------------------------
' 8. ByRef scalar parameters
' -----------------------------------------------------------------------
Print "--- 8. ByRef scalar ---"

Sub SwapInt(ByRef p% As Integer, ByRef q% As Integer)
    Dim tmp% As Integer
    tmp% = p%
    p% = q%
    q% = tmp%
End Sub

Dim u% As Integer
Dim v% As Integer
u% = 3
v% = 9
Call SwapInt(u%, v%)
AssertEq "swap u%=9", Str$(u%), "9"
AssertEq "swap v%=3", Str$(v%), "3"

' -----------------------------------------------------------------------
' 9. ByRef string parameter
' -----------------------------------------------------------------------
Print "--- 9. ByRef string ---"

Sub Shout(ByRef s$ As String)
    s$ = UCase$(s$) + "!!!"
End Sub

Dim msg$ As String
msg$ = "hello"
Call Shout(msg$)
AssertEq "shout", msg$, "HELLO!!!"

' -----------------------------------------------------------------------
' 10. Function calling another function
' -----------------------------------------------------------------------
Print "--- 10. Chained function calls ---"

Function DoubleSquare%(n% As Integer)
    DoubleSquare% = Square%(Square%(n%))
End Function

AssertEq "double_square(2) = sq(sq(2)) = sq(4) = 16", Str$(DoubleSquare%(2)), "16"
AssertEq "double_square(3) = sq(sq(3)) = sq(9) = 81", Str$(DoubleSquare%(3)), "81"

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
