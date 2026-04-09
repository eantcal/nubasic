' test_control_flow.bas
' Tests:
'   1. IF / THEN / ELSE / END IF
'   2. Nested IF
'   3. FOR / NEXT basic
'   4. FOR / NEXT with STEP
'   5. FOR / NEXT with EXIT FOR
'   6. WHILE / WEND
'   7. DO / LOOP WHILE
'   8. GOSUB / RETURN
'   9. Nested loops (accumulator)
'  10. ON GOTO (dispatch table)

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
' 1. IF / THEN / ELSE / END IF
' -----------------------------------------------------------------------
Print "--- 1. IF/THEN/ELSE ---"

Dim a% As Integer
a% = 10
Dim r$ As String

If a% > 5 Then
    r$ = "gt"
Else
    r$ = "le"
End If
AssertEq "a%>5  -> gt", r$, "gt"

If a% > 20 Then
    r$ = "gt"
Else
    r$ = "le"
End If
AssertEq "a%>20 -> le", r$, "le"

' -----------------------------------------------------------------------
' 2. Nested IF
' -----------------------------------------------------------------------
Print "--- 2. Nested IF ---"

Dim x% As Integer
x% = 15
r$ = ""

If x% > 10 Then
    If x% > 20 Then
        r$ = "big"
    Else
        r$ = "medium"
    End If
Else
    r$ = "small"
End If
AssertEq "nested IF x%=15", r$, "medium"

x% = 5
If x% > 10 Then
    r$ = "big"
Else
    If x% > 2 Then
        r$ = "medium"
    Else
        r$ = "small"
    End If
End If
AssertEq "nested IF x%=5", r$, "medium"

' -----------------------------------------------------------------------
' 3. FOR / NEXT basic
' -----------------------------------------------------------------------
Print "--- 3. FOR/NEXT basic ---"

Dim sum% As Integer
sum% = 0
Dim i% As Integer
For i% = 1 To 5
    sum% = sum% + i%
Next i%
AssertEq "sum 1..5", Str$(sum%), "15"

' -----------------------------------------------------------------------
' 4. FOR / NEXT with STEP
' -----------------------------------------------------------------------
Print "--- 4. FOR/NEXT STEP ---"

sum% = 0
For i% = 0 To 10 Step 2
    sum% = sum% + i%
Next i%
AssertEq "sum evens 0..10", Str$(sum%), "30"

sum% = 0
For i% = 5 To 1 Step -1
    sum% = sum% + i%
Next i%
AssertEq "countdown 5..1", Str$(sum%), "15"

' -----------------------------------------------------------------------
' 5. FOR / NEXT with EXIT FOR
' -----------------------------------------------------------------------
Print "--- 5. EXIT FOR ---"

Dim found% As Integer
found% = -1
For i% = 1 To 10
    If i% = 6 Then
        found% = i%
        Exit For
    End If
Next i%
AssertEq "exit for at 6", Str$(found%), "6"
' After EXIT FOR the loop variable has already been incremented to the next value
AssertEq "i% after exit", Str$(i%), "7"

' -----------------------------------------------------------------------
' 6. WHILE / WEND
' -----------------------------------------------------------------------
Print "--- 6. WHILE/WEND ---"

Dim n% As Integer
n% = 1
sum% = 0
While n% <= 5
    sum% = sum% + n%
    n% = n% + 1
Wend
AssertEq "while sum 1..5", Str$(sum%), "15"

' EXIT WHILE
n% = 1
sum% = 0
While n% <= 100
    If n% > 5 Then Exit While
    sum% = sum% + n%
    n% = n% + 1
Wend
AssertEq "exit while at 6", Str$(sum%), "15"

' -----------------------------------------------------------------------
' 7. DO / LOOP WHILE
' -----------------------------------------------------------------------
Print "--- 7. DO/LOOP WHILE ---"

n% = 1
sum% = 0
Do
    sum% = sum% + n%
    n% = n% + 1
Loop While n% <= 5
AssertEq "do/loop sum 1..5", Str$(sum%), "15"

' Body executes at least once even when condition is false
n% = 10
sum% = 0
Do
    sum% = sum% + 1
    n% = n% + 1
Loop While n% < 5
AssertEq "do/loop runs once", Str$(sum%), "1"

' -----------------------------------------------------------------------
' 8. GOSUB / RETURN
' -----------------------------------------------------------------------
Print "--- 8. GOSUB/RETURN ---"

Dim gsub_res% As Integer
gsub_res% = 0
GoSub ComputeDouble
AssertEq "gosub double 7", Str$(gsub_res%), "14"
GoTo SkipSub

ComputeDouble:
    gsub_res% = 7 * 2
    Return

SkipSub:

' -----------------------------------------------------------------------
' 9. Nested loops (accumulator)
' -----------------------------------------------------------------------
Print "--- 9. Nested loops ---"

Dim cnt% As Integer
cnt% = 0
Dim ii% As Integer
Dim jj% As Integer
For ii% = 1 To 3
    For jj% = 1 To 3
        cnt% = cnt% + 1
    Next jj%
Next ii%
AssertEq "nested 3x3 iterations", Str$(cnt%), "9"

' -----------------------------------------------------------------------
' 10. ON GOTO — label list, inline label+statement, out-of-range fall-through
' -----------------------------------------------------------------------
Print "--- 10. ON GOTO ---"

Dim selector% As Integer
r$ = ""
' ON GOTO is 0-based: selector%=0 -> Case1, 1 -> Case2, 2 -> Case3
selector% = 0 : On selector% GoTo Case1, Case2, Case3
Case1: r$ = "one"   : GoTo EndSel0
Case2: r$ = "two"   : GoTo EndSel0
Case3: r$ = "three"
EndSel0:
AssertEq "on goto idx=0 -> one", r$, "one"

r$ = ""
selector% = 1 : On selector% GoTo Case1b, Case2b, Case3b
Case1b: r$ = "one"   : GoTo EndSel1
Case2b: r$ = "two"   : GoTo EndSel1
Case3b: r$ = "three"
EndSel1:
AssertEq "on goto idx=1 -> two", r$, "two"

r$ = ""
selector% = 2 : On selector% GoTo Case1c, Case2c, Case3c
Case1c: r$ = "one"   : GoTo EndSel2
Case2c: r$ = "two"   : GoTo EndSel2
Case3c: r$ = "three"
EndSel2:
AssertEq "on goto idx=2 -> three", r$, "three"

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
