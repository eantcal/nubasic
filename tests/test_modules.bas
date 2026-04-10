' test_modules.bas
' Tests:
'   1.  Qualified built-in calls in modern mode
'   2.  Using imports unqualified names
'   3.  Include loads built-in modules into the current namespace
'   4.  Legacy mode restores the classic global API surface

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

' AssertFuzzy uses only qualified names so it works in any syntax mode
Sub AssertFuzzy(label$, a As Double, b As Double)
    Dim diff As Double
    diff = math::abs(a - b)
    Dim tol As Double
    tol = math::abs(b) * 0.000001
    If tol < 0.000001 Then tol = 0.000001

    If diff < tol Then
        Print "  PASS  " + label$
        pass% = pass% + 1
    Else
        Print "  FAIL  " + label$ + "  got=" + string::str$(a) + "  expected=" + string::str$(b)
        fail% = fail% + 1
    End If
End Sub

Print "--- 1. Qualified built-ins in modern mode ---"

Syntax Modern

AssertFuzzy "math::sin(0)", math::sin(0.0), 0.0
AssertEq "string::left$('Hello', 2)", string::left$("Hello", 2), "He"

Dim nums(3) As Integer
nums(0) = 10
nums(1) = 20
nums(2) = 30
AssertEq "runtime::sizeof(nums)", string::str$(runtime::sizeof(nums)), "3"

Print "--- 2. Using imports unqualified names ---"

Using math
Using string

AssertFuzzy "Sin after Using math", Sin(0.0), 0.0
AssertEq "Left$ after Using string", Left$("World", 3), "Wor"

Print "--- 3. Include module aliases ---"

Syntax Modern
Include "math"
Include "string"

AssertFuzzy "Cos after Include math", Cos(0.0), 1.0
AssertEq "Right$ after Include string", Right$("World", 2), "ld"

Print "--- 4. Legacy mode restores globals ---"

Syntax Legacy

AssertEq "Len in legacy mode", Str$(Len("abc")), "3"
AssertFuzzy "Pi in legacy mode", Pi(), 3.141592653589793

Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
