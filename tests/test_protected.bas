' test_protected.bas
' Tests for Protected access modifier:
'   - protected member accessible from declaring class
'   - protected member accessible from direct derived class
'   - protected member accessible through multi-level inheritance chain

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

' --- Base class with protected field and method ---
Class Animal
    Protected Name As String
    Protected Kind As String

    Protected Sub Describe()
        Me.Kind = "animal"
    End Sub

    Public Sub Init(n As String)
        Me.Name = n
        Me.Describe()
    End Sub

    Public Function GetInfo$()
        GetInfo$ = Me.Kind + ":" + Me.Name
    End Function
End Class

' --- Direct derived class accesses protected members ---
Class Dog
    Inherits Animal

    Public Overridable Sub Describe()
        Me.Kind = "dog"
    End Sub

    Public Sub Rename(n As String)
        Me.Name = n
    End Sub

    Public Function GetName$()
        GetName$ = Me.Name
    End Function
End Class

' --- Multi-level derived class ---
Class Poodle
    Inherits Dog

    Public Overrides Sub Describe()
        Me.Kind = "poodle"
    End Sub

    Public Function GetKind$()
        GetKind$ = Me.Kind
    End Function
End Class

Print "--- Protected: base class accesses its own protected member ---"

Dim a As New Animal()
a.Init("Leo")
AssertEq "base sets protected field via public method", a.GetInfo$(), "animal:Leo"

Print ""
Print "--- Protected: derived class reads/writes inherited protected field ---"

Dim d As New Dog()
d.Init("Rex")
AssertEq "derived Init uses protected field", d.GetInfo$(), "dog:Rex"
d.Rename("Buddy")
AssertEq "derived method writes protected Name", d.GetName$(), "Buddy"

Print ""
Print "--- Protected: two-level derived class accesses protected field ---"

Dim p As New Poodle()
p.Init("Fifi")
AssertEq "grandchild Init uses protected field", p.GetInfo$(), "poodle:Fifi"
AssertEq "grandchild reads protected Kind", p.GetKind$(), "poodle"

Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
