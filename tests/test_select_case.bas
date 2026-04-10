' test_select_case.bas
' Tests:
'   1. Basic integer matching
'   2. Multi-value list (Case 1, 3, 5)
'   3. Range  (Case 1 To 10)
'   4. IS comparison (Case Is > 10)
'   5. Case Else fallback
'   6. String matching
'   7. No-match without Case Else (block skipped)
'   8. Multiple consecutive SELECT CASEs
'   9. Nested SELECT CASE

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
' 1. Basic integer matching
' -----------------------------------------------------------------------
Print "--- 1. Basic integer matching ---"

Dim x% As Integer
Dim r$ As String

x% = 2
r$ = ""
Select Case x%
    Case 1
        r$ = "one"
    Case 2
        r$ = "two"
    Case 3
        r$ = "three"
End Select
AssertEq "x%=2 -> two", r$, "two"

x% = 1
r$ = ""
Select Case x%
    Case 1
        r$ = "one"
    Case 2
        r$ = "two"
End Select
AssertEq "x%=1 -> one", r$, "one"

' -----------------------------------------------------------------------
' 2. Multi-value list
' -----------------------------------------------------------------------
Print "--- 2. Multi-value list ---"

x% = 3
r$ = ""
Select Case x%
    Case 1, 3, 5
        r$ = "odd135"
    Case 2, 4, 6
        r$ = "even246"
    Case Else
        r$ = "other"
End Select
AssertEq "x%=3 -> odd135", r$, "odd135"

x% = 4
r$ = ""
Select Case x%
    Case 1, 3, 5
        r$ = "odd135"
    Case 2, 4, 6
        r$ = "even246"
    Case Else
        r$ = "other"
End Select
AssertEq "x%=4 -> even246", r$, "even246"

' -----------------------------------------------------------------------
' 3. Range (Case X To Y)
' -----------------------------------------------------------------------
Print "--- 3. Range ---"

x% = 7
r$ = ""
Select Case x%
    Case 1 To 5
        r$ = "low"
    Case 6 To 10
        r$ = "mid"
    Case 11 To 20
        r$ = "high"
End Select
AssertEq "x%=7 -> mid", r$, "mid"

x% = 1
r$ = ""
Select Case x%
    Case 1 To 5
        r$ = "low"
    Case 6 To 10
        r$ = "mid"
End Select
AssertEq "x%=1 -> low (boundary)", r$, "low"

x% = 10
r$ = ""
Select Case x%
    Case 1 To 5
        r$ = "low"
    Case 6 To 10
        r$ = "mid"
End Select
AssertEq "x%=10 -> mid (boundary)", r$, "mid"

' -----------------------------------------------------------------------
' 4. IS comparison
' -----------------------------------------------------------------------
Print "--- 4. IS comparison ---"

x% = 15
r$ = ""
Select Case x%
    Case Is < 0
        r$ = "neg"
    Case Is = 0
        r$ = "zero"
    Case Is > 0
        r$ = "pos"
End Select
AssertEq "x%=15 Is > 0 -> pos", r$, "pos"

x% = -3
r$ = ""
Select Case x%
    Case Is < 0
        r$ = "neg"
    Case Is >= 0
        r$ = "nonneg"
End Select
AssertEq "x%=-3 Is < 0 -> neg", r$, "neg"

' -----------------------------------------------------------------------
' 5. Case Else fallback
' -----------------------------------------------------------------------
Print "--- 5. Case Else ---"

x% = 99
r$ = ""
Select Case x%
    Case 1
        r$ = "one"
    Case 2
        r$ = "two"
    Case Else
        r$ = "other"
End Select
AssertEq "x%=99 -> other", r$, "other"

' -----------------------------------------------------------------------
' 6. String matching
' -----------------------------------------------------------------------
Print "--- 6. String matching ---"

Dim s$ As String
s$ = "hello"
r$ = ""
Select Case s$
    Case "world"
        r$ = "world"
    Case "hello"
        r$ = "hello"
    Case Else
        r$ = "unknown"
End Select
AssertEq "s$=hello -> hello", r$, "hello"

s$ = "foo"
r$ = ""
Select Case s$
    Case "bar"
        r$ = "bar"
    Case Else
        r$ = "other"
End Select
AssertEq "s$=foo -> other", r$, "other"

' -----------------------------------------------------------------------
' 7. No-match without Case Else (block skipped cleanly)
' -----------------------------------------------------------------------
Print "--- 7. No-match, no Else ---"

x% = 99
r$ = "unchanged"
Select Case x%
    Case 1
        r$ = "one"
    Case 2
        r$ = "two"
End Select
AssertEq "no match no else -> unchanged", r$, "unchanged"

' -----------------------------------------------------------------------
' 8. Multiple consecutive SELECT CASEs
' -----------------------------------------------------------------------
Print "--- 8. Multiple consecutive ---"

Dim a% As Integer
Dim b% As Integer
a% = 10
b% = 20

r$ = ""
Select Case a%
    Case 10
        r$ = "a10"
    Case Else
        r$ = "anot"
End Select
AssertEq "a%=10 -> a10", r$, "a10"

r$ = ""
Select Case b%
    Case 10
        r$ = "b10"
    Case 20
        r$ = "b20"
    Case Else
        r$ = "bnot"
End Select
AssertEq "b%=20 -> b20", r$, "b20"

' -----------------------------------------------------------------------
' 9. Nested SELECT CASE
' -----------------------------------------------------------------------
Print "--- 9. Nested ---"

x% = 2
Dim y% As Integer
y% = 5
r$ = ""

Select Case x%
    Case 1
        r$ = "x1"
    Case 2
        Select Case y%
            Case 5
                r$ = "x2y5"
            Case Else
                r$ = "x2yother"
        End Select
    Case Else
        r$ = "xother"
End Select
AssertEq "nested x=2,y=5 -> x2y5", r$, "x2y5"

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
