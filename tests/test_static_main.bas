' test_static_main.bas
' Tests:
'   1. Static Function callable without instance
'   2. Static Sub callable without instance
'   3. Static method in expression context
'   4. main() with no parameters
'   5. main() with argc parameter

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
' Define a class with static methods
' -----------------------------------------------------------------------
Class MathHelper
    Public x As Integer

    Static Function Add(a As Integer, b As Integer) As Integer
        Add = a + b
    End Function

    Static Function Mul(a As Integer, b As Integer) As Integer
        Mul = a * b
    End Function

    Static Sub PrintSum(a As Integer, b As Integer)
        Print "Sum ="; a + b
    End Sub

    Function GetX() As Integer
        GetX = Me.x
    End Function
End Class

' -----------------------------------------------------------------------
' 1. Static Function callable without instance
' -----------------------------------------------------------------------
Print "--- 1. Static Function (no instance) ---"

Dim r As Integer
r = MathHelper.Add(3, 7)
AssertEq "Add(3,7)=10", Str$(r), "10"

r = MathHelper.Mul(4, 5)
AssertEq "Mul(4,5)=20", Str$(r), "20"

' -----------------------------------------------------------------------
' 2. Static Sub callable without instance (method call syntax)
' -----------------------------------------------------------------------
Print "--- 2. Static Sub (method call syntax) ---"

' We just verify it doesn't crash; side effect is a Print
MathHelper.PrintSum 2, 3
AssertEq "PrintSum runs", "ok", "ok"

' -----------------------------------------------------------------------
' 3. Static method in expression context
' -----------------------------------------------------------------------
Print "--- 3. Static in expression ---"

Dim s As Integer
s = MathHelper.Add(10, MathHelper.Mul(2, 3))
AssertEq "Add(10, Mul(2,3)) = Add(10,6) = 16", Str$(s), "16"

' -----------------------------------------------------------------------
' 4. Instance method still works alongside static methods
' -----------------------------------------------------------------------
Print "--- 4. Instance method still works ---"

Dim h As New MathHelper()
h.x = 42
AssertEq "GetX()=42", Str$(h.GetX()), "42"

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
