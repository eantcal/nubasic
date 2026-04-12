' test_mybase.bas — MyBase dispatch to base-class methods

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

' --- 1. MyBase.Sub() — derived Sub extends base Sub ---

Class Animal
    Public name$ As String
    Sub Speak()
        Me.name$ = Me.name$ + "[animal]"
    End Sub
End Class

Class Dog
    Inherits Animal
    Sub Speak()
        MyBase.Speak()
        Me.name$ = Me.name$ + "[dog]"
    End Sub
End Class

Print "--- 1. MyBase.Sub() ---"
Dim d As New Dog()
d.name$ = ""
d.Speak()
AssertEq "Dog.Speak chain", d.name$, "[animal][dog]"

' --- 2. MyBase.Function() in expression context ---

Class Shape
    Public color$ As String
    Function Describe$()
        Describe$ = "Shape(" + Me.color$ + ")"
    End Function
End Class

Class Circle
    Inherits Shape
    Public radius% As Integer
    Function Describe$()
        Describe$ = MyBase.Describe$() + "+Circle(r=" + Str$(Me.radius%) + ")"
    End Function
End Class

Print "--- 2. MyBase.Function() in expression ---"
Dim c As New Circle()
c.color$ = "red"
c.radius% = 5
AssertEq "Circle.Describe$", c.Describe$(), "Shape(red)+Circle(r=5)"

' --- 3. Two-level chain: grandchild calls parent, parent calls grandparent ---

Class A
    Public v% As Integer
    Sub Init()
        Me.v% = 1
    End Sub
End Class

Class B
    Inherits A
    Sub Init()
        MyBase.Init()
        Me.v% = Me.v% + 10
    End Sub
End Class

Class C
    Inherits B
    Sub Init()
        MyBase.Init()
        Me.v% = Me.v% + 100
    End Sub
End Class

Print "--- 3. Two-level MyBase chain ---"
Dim obj As New C()
obj.Init()
AssertEq "C.Init chain (1+10+100)", Str$(obj.v%), "111"

' --- 4. MyBase with argument ---

Class Adder
    Public total% As Integer
    Sub Add(n% As Integer)
        Me.total% = Me.total% + n%
    End Sub
End Class

Class DoubleAdder
    Inherits Adder
    Sub Add(n% As Integer)
        MyBase.Add(n%)
        MyBase.Add(n%)
    End Sub
End Class

Print "--- 4. MyBase with argument ---"
Dim da As New DoubleAdder()
da.Add(3)
AssertEq "DoubleAdder.Add(3)", Str$(da.total%), "6"

' --- Results ---

Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
