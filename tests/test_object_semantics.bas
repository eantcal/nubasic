' test_object_semantics.bas
' Tests true object reference semantics for Class instances.

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

Class Box
    Public Value% As Integer

    Public Sub SetValue(n% As Integer)
        Me.Value% = n%
    End Sub

    Public Function GetValue%()
        GetValue% = Me.Value%
    End Function
End Class

Class Holder
    Public Item As Box
End Class

Class Node
    Public Value% As Integer
    Public Link As Node
End Class

Class Animal
    Public Tag$ As String

    Overridable Sub Mark()
        Me.Tag$ = "[animal]"
    End Sub
End Class

Class Dog
    Inherits Animal

    Overrides Sub Mark()
        Me.Tag$ = "[dog]"
    End Sub
End Class

Sub TouchByVal(obj As Box)
    obj.SetValue(12)
End Sub

Function SameBox(obj As Box) As Box
    SameBox = obj
End Function

Print "--- 1. Assignment aliases object identity ---"
Dim a As New Box()
Dim b As Box
AssertEq "plain class variable starts Nothing", Str$(b = Nothing), "1"
b = a
a.SetValue(7)
AssertEq "alias observes mutation", Str$(b.GetValue%()), "7"
b.SetValue(9)
AssertEq "source observes alias mutation", Str$(a.GetValue%()), "9"
AssertEq "same reference compares equal", Str$(a = b), "1"

Dim c As New Box()
AssertEq "different instances compare different", Str$(a <> c), "1"

Print "--- 2. Nothing assignment and comparison ---"
b = Nothing
AssertEq "assigned Nothing compares equal", Str$(b = Nothing), "1"
AssertEq "source still alive after alias cleared", Str$(a.GetValue%()), "9"

Print "--- 3. ByVal object parameter copies the reference ---"
Call TouchByVal(a)
AssertEq "ByVal object mutation reaches caller", Str$(a.GetValue%()), "12"

Print "--- 4. Object return preserves identity ---"
Dim r As Box
r = SameBox(a)
r.SetValue(17)
AssertEq "returned object aliases source", Str$(a.GetValue%()), "17"

Print "--- 5. Object fields are distinct per owner but assignable as refs ---"
Dim h1 As New Holder()
Dim h2 As New Holder()
AssertEq "object field starts Nothing", Str$(h1.Item = Nothing), "1"

Dim item1 As New Box()
Dim item2 As New Box()
h1.Item = item1
h2.Item = item2
h1.Item.SetValue(3)
h2.Item.SetValue(4)
AssertEq "h1 item independent", Str$(h1.Item.GetValue%()), "3"
AssertEq "h2 item independent", Str$(h2.Item.GetValue%()), "4"

Dim itemAlias As Box
itemAlias = h1.Item
itemAlias.SetValue(5)
AssertEq "field alias mutates owner field", Str$(h1.Item.GetValue%()), "5"

Print "--- 6. Self-referential object fields ---"
Dim node As New Node()
AssertEq "self reference field starts Nothing", Str$(node.Link = Nothing), "1"
node.Link = node
node.Link.Value% = 21
AssertEq "self reference aliases owner", Str$(node.Value%), "21"

Print "--- 7. Base-typed variable keeps runtime identity ---"
Dim d As New Dog()
Dim animalRef As Animal
animalRef = d
animalRef.Mark()
AssertEq "base reference dispatches runtime override", d.Tag$, "[dog]"

Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
