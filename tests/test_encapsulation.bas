' test_encapsulation.bas
' Positive tests for private member access from inside the declaring class.

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

Class Vault
    Private Secret As String

    Private Sub Wrap()
        Me.Secret = "[" + Me.Secret + "]"
    End Sub

    Public Sub SetSecret(s As String)
        Me.Secret = s
        Me.Wrap()
    End Sub

    Public Function Reveal$()
        Reveal$ = Me.Secret
    End Function

    Private Static Function Hidden() As Integer
        Hidden = 41
    End Function

    Public Static Function Answer() As Integer
        Answer = Vault.Hidden() + 1
    End Function
End Class

Class BaseBox
    Public Exposed As String
    Private Token As String

    Public Sub SetToken(s As String)
        Me.Token = s
    End Sub

    Public Sub PublishToken()
        Me.Exposed = Me.Token
    End Sub
End Class

Class ChildBox
    Inherits BaseBox

    Public Sub Touch()
        MyBase.SetToken("child")
        MyBase.PublishToken()
    End Sub
End Class

Print "--- Encapsulation: allowed internal access ---"

Dim v As New Vault()
v.SetSecret("alpha")
AssertEq "private field through public methods", v.Reveal$(), "[alpha]"
AssertEq "private static helper through public static", Str$(Vault.Answer()), "42"

Dim c As New ChildBox()
c.Touch()
AssertEq "base private field accessed by base method on derived instance", c.Exposed, "child"

Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
