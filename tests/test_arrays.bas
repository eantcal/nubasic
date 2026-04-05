' test_arrays.bas
' Tests:
'   1. Dim 1-D Integer array
'   2. Dim 1-D Double array
'   3. Dim 1-D String array
'   4. SizeOf
'   5. ReDim
'   6. Array as accumulator (sum via loop)
'   7. Matrix simulation with 1-D array + index arithmetic
'   8. Vector built-in: conv / sizeof

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

Function FuzzyEq%(a As Double, b As Double)
    If Abs(a - b) < 0.000001 Then
        FuzzyEq% = 1
    Else
        FuzzyEq% = 0
    End If
End Function

Sub AssertFuzzy(label$, a As Double, b As Double)
    If FuzzyEq%(a, b) Then
        Print "  PASS  " + label$
        pass% = pass% + 1
    Else
        Print "  FAIL  " + label$ + "  got=" + Str$(a) + "  expected=" + Str$(b)
        fail% = fail% + 1
    End If
End Sub

' -----------------------------------------------------------------------
' 1. Dim 1-D Integer array
' -----------------------------------------------------------------------
Print "--- 1. Integer array ---"

Dim ia%(5) As Integer
Dim i% As Integer
For i% = 0 To 4
    ia%(i%) = i% * 2
Next i%

AssertEq "ia%(0)", Str$(ia%(0)), "0"
AssertEq "ia%(2)", Str$(ia%(2)), "4"
AssertEq "ia%(4)", Str$(ia%(4)), "8"

' -----------------------------------------------------------------------
' 2. Dim 1-D Double array
' -----------------------------------------------------------------------
Print "--- 2. Double array ---"

Dim da(4) As Double
da(0) = 1.5
da(1) = 2.5
da(2) = 3.5
da(3) = 4.5

AssertFuzzy "da(0)", da(0), 1.5
AssertFuzzy "da(3)", da(3), 4.5

' -----------------------------------------------------------------------
' 3. Dim 1-D String array
' -----------------------------------------------------------------------
Print "--- 3. String array ---"

Dim sa$(3) As String
sa$(0) = "alpha"
sa$(1) = "beta"
sa$(2) = "gamma"

AssertEq "sa$(0)", sa$(0), "alpha"
AssertEq "sa$(1)", sa$(1), "beta"
AssertEq "sa$(2)", sa$(2), "gamma"

' -----------------------------------------------------------------------
' 4. SizeOf
' -----------------------------------------------------------------------
Print "--- 4. SizeOf ---"

Dim arr%(10) As Integer
AssertEq "sizeof(arr% 10)", Str$(SizeOf(arr%)), "10"

Dim brr(3) As Double
AssertEq "sizeof(brr 3)",   Str$(SizeOf(brr)),  "3"

' -----------------------------------------------------------------------
' 5. ReDim
' -----------------------------------------------------------------------
Print "--- 5. ReDim ---"

Dim dyn%(3) As Integer
dyn%(0) = 99
AssertEq "dyn%(0) before redim", Str$(dyn%(0)), "99"
AssertEq "sizeof before redim",  Str$(SizeOf(dyn%)), "3"

ReDim dyn%(6) As Integer
AssertEq "sizeof after redim",   Str$(SizeOf(dyn%)), "6"

' -----------------------------------------------------------------------
' 6. Array as accumulator (sum via loop)
' -----------------------------------------------------------------------
Print "--- 6. Array sum ---"

Dim nums%(5) As Integer
Dim k% As Integer
For k% = 0 To 4
    nums%(k%) = k% + 1
Next k%

Dim total% As Integer
total% = 0
For k% = 0 To SizeOf(nums%) - 1
    total% = total% + nums%(k%)
Next k%
AssertEq "sum 1..5", Str$(total%), "15"

' -----------------------------------------------------------------------
' 7. Matrix simulation (3x3) with 1-D array
' -----------------------------------------------------------------------
Print "--- 7. Matrix (3x3) ---"

' index = row*3 + col
Dim mat%(9) As Integer
Dim row% As Integer
Dim col% As Integer
For row% = 0 To 2
    For col% = 0 To 2
        mat%(row% * 3 + col%) = row% * 10 + col%
    Next col%
Next row%

AssertEq "mat[0][0]", Str$(mat%(0 * 3 + 0)), "0"
AssertEq "mat[1][2]", Str$(mat%(1 * 3 + 2)), "12"
AssertEq "mat[2][1]", Str$(mat%(2 * 3 + 1)), "21"

' -----------------------------------------------------------------------
' 8. Conv / SizeOf on Double vectors
' -----------------------------------------------------------------------
Print "--- 8. Conv ---"

Dim v1(3) As Double
Dim v2(2) As Double
v1(0) = 1.0
v1(1) = 0.0
v1(2) = 0.0

v2(0) = 1.0
v2(1) = 2.0

' Convolution of [1,0,0] with [1,2]: result(0)=1, result(1)=2
Dim res(4) As Double
res = Conv(v1, v2, SizeOf(v1), SizeOf(v2))

AssertFuzzy "conv(0) = 1", res(0), 1.0
AssertFuzzy "conv(1) = 2", res(1), 2.0

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
