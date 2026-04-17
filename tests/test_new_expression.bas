' test_new_expression.bas
' Tests first-class New expressions for class references.

Syntax Modern
Using string

Dim pass% As Integer
Dim fail% As Integer
pass% = 0
fail% = 0

Sub AssertEq(label As String, got As String, expected As String)
    If got = expected Then
        Print "  PASS  " + label
        pass% = pass% + 1
    Else
        Print "  FAIL  " + label + "  got='" + got + "'  expected='" + expected + "'"
        fail% = fail% + 1
    End If
End Sub

Class Box
    Public Value% As Integer

    Public Sub New()
        Me.Value% = 1
    End Sub

    Public Sub SetValue(n% As Integer)
        Me.Value% = n%
    End Sub

    Public Function GetValue%()
        GetValue% = Me.Value%
    End Function
End Class

Class NamedBox
    Public Name$ As String

    Public Sub New(name As String)
        Me.Name$ = name
    End Sub
End Class

Class Holder
    Public Item As Box
End Class

Class Animal
    Public Kind$ As String

    Public Overridable Sub Mark()
        Me.Kind$ = "animal"
    End Sub
End Class

Class Dog
    Inherits Animal

    Public Overrides Sub Mark()
        Me.Kind$ = "dog"
    End Sub
End Class

Class Node
    Public Value% As Integer
    Public Link As Node
End Class

Function ReadValue%(obj As Box)
    obj.SetValue(7)
    ReadValue% = obj.GetValue%()
End Function

Function MakeBox(v% As Integer) As Box
    MakeBox = New Box()
    MakeBox.SetValue(v%)
End Function

Function MakeAnimal() As Animal
    MakeAnimal = New Dog()
End Function

Print "--- 1. Direct assignment ---"
Dim b As Box
b = New Box()
AssertEq "New Box() runs default constructor", Str$(b.GetValue%()), "1"

Print "--- 2. Constructor args ---"
Dim named As NamedBox
named = New NamedBox("Ada")
AssertEq "New NamedBox(arg)", named.Name$, "Ada"

Print "--- 3. Field assignment ---"
Dim h As New Holder()
h.Item = New Box()
h.Item.SetValue(5)
AssertEq "field = New Box()", Str$(h.Item.GetValue%()), "5"

Print "--- 4. Inline parameter ---"
AssertEq "New Box() as parameter", Str$(ReadValue%(New Box())), "7"

Print "--- 5. Function return ---"
Dim made As Box
made = MakeBox(11)
AssertEq "function returns New object", Str$(made.GetValue%()), "11"

Print "--- 6. Base reference ---"
Dim a As Animal
a = New Dog()
a.Mark()
AssertEq "base ref gets derived object", a.Kind$, "dog"

Dim madeAnimal As Animal
madeAnimal = MakeAnimal()
madeAnimal.Mark()
AssertEq "function returns derived as base", madeAnimal.Kind$, "dog"

Print "--- 7. Self-referential field ---"
Dim n As New Node()
n.Link = New Node()
n.Link.Value% = 19
AssertEq "self-style field gets New object", Str$(n.Link.Value%), "19"

Print "--- 8. No-paren New ---"
Dim bare As Box
bare = New Box
AssertEq "New Box without parentheses", Str$(bare.GetValue%()), "1"

Print "--- 9. Temporary member access ---"
AssertEq "New Box().Method()", Str$(New Box().GetValue%()), "1"
AssertEq "New NamedBox(arg).Field", New NamedBox("Lin").Name$, "Lin"

Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
