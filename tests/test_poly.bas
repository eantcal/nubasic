' test_poly.bas — Runtime polymorphism (virtual dispatch) tests

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
' --- 1. Direct dispatch: derived class overrides base ---
' -----------------------------------------------------------------------

Class Shape
    Public tag$ As String
    Overridable Sub Describe()
        Me.tag$ = Me.tag$ + "[shape]"
    End Sub
End Class

Class Circle
    Inherits Shape
    Overrides Sub Describe()
        Me.tag$ = Me.tag$ + "[circle]"
    End Sub
End Class

Class Square
    Inherits Shape
    Overrides Sub Describe()
        Me.tag$ = Me.tag$ + "[square]"
    End Sub
End Class

Print "--- 1. Direct dispatch ---"
Dim sh As New Shape()
sh.tag$ = ""
sh.Describe()
AssertEq "Shape.Describe", sh.tag$, "[shape]"

Dim ci As New Circle()
ci.tag$ = ""
ci.Describe()
AssertEq "Circle.Describe", ci.tag$, "[circle]"

Dim sq As New Square()
sq.tag$ = ""
sq.Describe()
AssertEq "Square.Describe", sq.tag$, "[square]"

' -----------------------------------------------------------------------
' --- 2. Inherited method (no override): falls through to base ---
' -----------------------------------------------------------------------

Class Animal
    Public name$ As String
    Overridable Sub Speak()
        Me.name$ = Me.name$ + "[animal]"
    End Sub
    Sub Breathe()
        Me.name$ = Me.name$ + "[breathe]"
    End Sub
End Class

Class Cat
    Inherits Animal
    Overrides Sub Speak()
        Me.name$ = Me.name$ + "[meow]"
    End Sub
    ' No Breathe override — inherits Animal.Breathe
End Class

Print "--- 2. Inherited fallback (no override) ---"
Dim c As New Cat()
c.name$ = ""
c.Breathe()
AssertEq "Cat inherits Animal.Breathe", c.name$, "[breathe]"

' -----------------------------------------------------------------------
' --- 3. Polymorphic dispatch through Sub parameter ---
'        Passing a derived instance to a Sub typed as base:
'        the runtime type must be used for dispatch.
' -----------------------------------------------------------------------

Sub MakeDescribe(ByRef obj As Shape)
    obj.Describe()
End Sub

Print "--- 3. Polymorphic dispatch through Sub parameter ---"
Dim ci2 As New Circle()
ci2.tag$ = ""
Call MakeDescribe(ci2)
AssertEq "Circle passed as Shape → Circle.Describe", ci2.tag$, "[circle]"

Dim sq2 As New Square()
sq2.tag$ = ""
Call MakeDescribe(sq2)
AssertEq "Square passed as Shape → Square.Describe", sq2.tag$, "[square]"

Dim sh2 As New Shape()
sh2.tag$ = ""
Call MakeDescribe(sh2)
AssertEq "Shape passed as Shape → Shape.Describe", sh2.tag$, "[shape]"

' -----------------------------------------------------------------------
' --- 4. Two-level inheritance chain: override at each level ---
' -----------------------------------------------------------------------

Class A
    Public v$ As String
    Overridable Sub Tag()
        Me.v$ = Me.v$ + "[A]"
    End Sub
End Class

Class B
    Inherits A
    Overrides Sub Tag()
        Me.v$ = Me.v$ + "[B]"
    End Sub
End Class

Class C
    Inherits B
    Overrides Sub Tag()
        Me.v$ = Me.v$ + "[C]"
    End Sub
End Class

Print "--- 4. Two-level override chain ---"
Dim a As New A()
a.v$ = ""
a.Tag()
AssertEq "A.Tag", a.v$, "[A]"

Dim b As New B()
b.v$ = ""
b.Tag()
AssertEq "B.Tag", b.v$, "[B]"

Dim cc As New C()
cc.v$ = ""
cc.Tag()
AssertEq "C.Tag", cc.v$, "[C]"

' -----------------------------------------------------------------------
' --- 5. Overridable Function in expression ---
' -----------------------------------------------------------------------

Class Formatter
    Public prefix$ As String
    Overridable Function Format$()
        Format$ = Me.prefix$ + "base"
    End Function
End Class

Class FancyFormatter
    Inherits Formatter
    Overrides Function Format$()
        Format$ = Me.prefix$ + "fancy"
    End Function
End Class

Print "--- 5. Overridable Function (expression context) ---"
Dim ff As New FancyFormatter()
ff.prefix$ = ">> "
AssertEq "FancyFormatter.Format$", ff.Format$(), ">> fancy"

Dim bf As New Formatter()
bf.prefix$ = ">> "
AssertEq "Formatter.Format$", bf.Format$(), ">> base"

' -----------------------------------------------------------------------
' --- Results ---
' -----------------------------------------------------------------------

Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
