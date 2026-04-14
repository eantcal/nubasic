' test_oop_mutation.bas — object mutation via Sub and Function method calls

Syntax Modern
Using string

Dim pass% As Integer
Dim fail% As Integer
pass% = 0
fail% = 0

Sub AssertEq(label$ As String, got$ As String, expected$ As String)
    If got$ = expected$ Then
        Print "  PASS  " + label$
        pass% = pass% + 1
    Else
        Print "  FAIL  " + label$ + "  got='" + got$ + "'  expected='" + expected$ + "'"
        fail% = fail% + 1
    End If
End Sub

' --- 1. Simple Sub mutates a field via Me ---

Class Counter
    Public value% As Integer
    Sub Increment()
        Me.value% = Me.value% + 1
    End Sub
    Sub Add(n% As Integer)
        Me.value% = Me.value% + n%
    End Sub
End Class

Print "--- 1. Sub mutates field via Me ---"
Dim c As New Counter()
c.Increment()
AssertEq "after Increment", Str$(c.value%), "1"
c.Add(4)
AssertEq "after Add(4)", Str$(c.value%), "5"

' --- 2. Sub calls another method of the same class via Me ---

Class Stack
    Public top% As Integer
    Sub Push()
        Me.top% = Me.top% + 1
    End Sub
    Sub PushTwice()
        Me.Push()
        Me.Push()
    End Sub
End Class

Print "--- 2. Me.Method() inside a Sub ---"
Dim s As New Stack()
s.PushTwice()
AssertEq "top after PushTwice", Str$(s.top%), "2"

' --- 3. Function mutates Me AND returns a value (expression context) ---

Class Accumulator
    Public sum% As Integer
    Function AddAndGet%(n% As Integer)
        Me.sum% = Me.sum% + n%
        AddAndGet% = Me.sum%
    End Function
End Class

Print "--- 3. Function mutates Me in expression context ---"
Dim a As New Accumulator()
Dim r% As Integer
r% = a.AddAndGet%(10)
AssertEq "AddAndGet%(10) retval", Str$(r%), "10"
AssertEq "sum after first call",  Str$(a.sum%), "10"
r% = a.AddAndGet%(5)
AssertEq "AddAndGet%(5) retval",  Str$(r%), "15"
AssertEq "sum after second call", Str$(a.sum%), "15"

' --- 4. Sub on a nested struct field (obj.field.Method()) ---

Class Point
    Public x% As Integer
    Sub ShiftRight()
        Me.x% = Me.x% + 1
    End Sub
End Class

Class Line
    Public origin As Point
End Class

Print "--- 4. Method on nested struct field ---"
Dim ln As New Line()
Dim origin As New Point()
ln.origin = origin
ln.origin.ShiftRight()
AssertEq "origin.x% after ShiftRight", Str$(ln.origin.x%), "1"
ln.origin.ShiftRight()
AssertEq "origin.x% after 2nd ShiftRight", Str$(ln.origin.x%), "2"

' --- 5. Multiple objects are independent ---

Print "--- 5. Multiple objects are independent ---"
Dim c1 As New Counter()
Dim c2 As New Counter()
c1.Increment()
c1.Increment()
c2.Increment()
AssertEq "c1.value%", Str$(c1.value%), "2"
AssertEq "c2.value%", Str$(c2.value%), "1"

' --- Results ---

Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
