' test_struct_byref.bas
' Tests:
'   1. Struct definition and member access
'   2. Nested struct (struct as field of another struct)
'   3. ByRef struct parameter: field changes propagate back to caller
'   4. ByRef scalar parameters: changes propagate back to caller
'   5. ByRef string parameter: changes propagate back to caller
'   6. ByRef struct variable: field mutations via ByRef

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
' 1. Basic struct
' -----------------------------------------------------------------------
Print "--- 1. Basic struct ---"

Struct Point
x As Double
    y As Double
End Struct

Dim p As Point
p.x = 3.0
p.y = 4.0

AssertEq "p.x", Str$(p.x), "3"
AssertEq "p.y", Str$(p.y), "4"

' -----------------------------------------------------------------------
' 2. Nested struct (struct field of struct)
' -----------------------------------------------------------------------
Print "--- 2. Nested struct ---"

Struct Segment
a As Point
    b As Point
End Struct

Dim seg As Segment
seg.a.x = 1.0
seg.a.y = 2.0
seg.b.x = 5.0
seg.b.y = 6.0

AssertEq "seg.a.x", Str$(seg.a.x), "1"
AssertEq "seg.a.y", Str$(seg.a.y), "2"
AssertEq "seg.b.x", Str$(seg.b.x), "5"
AssertEq "seg.b.y", Str$(seg.b.y), "6"

' -----------------------------------------------------------------------
' 3. ByRef struct parameter: field mutations propagate back to caller
' -----------------------------------------------------------------------
Print "--- 3. ByRef struct field mutation ---"

Sub TranslatePoint(ByRef q As Point, dx As Double, dy As Double)
    q.x = q.x + dx
    q.y = q.y + dy
End Sub

Dim tp As Point
tp.x = 10.0
tp.y = 20.0
Call TranslatePoint(tp, 3.0, -5.0)

AssertEq "tp.x after Translate", Str$(tp.x), "13"
AssertEq "tp.y after Translate", Str$(tp.y), "15"

' -----------------------------------------------------------------------
' 4. ByRef scalar parameters
' -----------------------------------------------------------------------
Print "--- 4. ByRef scalar ---"

Sub Swap(ByRef a% As Integer, ByRef b% As Integer)
    Dim tmp% As Integer
    tmp% = a%
    a% = b%
    b% = tmp%
End Sub

Dim x% As Integer
Dim y% As Integer
x% = 7
y% = 42
Call Swap(x%, y%)

AssertEq "x% after Swap", Str$(x%), "42"
AssertEq "y% after Swap", Str$(y%), "7"

' -----------------------------------------------------------------------
' 5. ByRef string parameter
' -----------------------------------------------------------------------
Print "--- 5. ByRef string ---"

Sub AppendBang(ByRef s$ As String)
    s$ = s$ + "!"
End Sub

Dim msg$ As String
msg$ = "Hello"
Call AppendBang(msg$)
AssertEq "msg$ after AppendBang", msg$, "Hello!"

' -----------------------------------------------------------------------
' 6. ByRef struct: field mutations via ByRef parameter
' -----------------------------------------------------------------------
Print "--- 6. ByRef struct ---"

Sub MakeOrigin(ByRef pt As Point)
    pt.x = 0.0
    pt.y = 0.0
End Sub

Dim origin As Point
origin.x = 99.0
origin.y = 99.0
Call MakeOrigin(origin)

AssertEq "origin.x after MakeOrigin", Str$(origin.x), "0"
AssertEq "origin.y after MakeOrigin", Str$(origin.y), "0"

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
