' test_class.bas
' Tests for VB-style class support (Phase 1: encapsulation and information hiding)
'
' Tests:
'   1. Class definition and public field access
'   2. Method call with no arguments (reads/writes public fields via Me)
'   3. Private field: only accessible through methods
'   4. Method with a parameter sets a private field; getter returns it
'   5. Multiple independent class instances
'   6. Method that computes a result and returns via a public field
'   7. Chained method calls: set then get on same object
'   8. Integer and string public fields coexist correctly

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
' Class definitions
' -----------------------------------------------------------------------

Class Counter
    Public Cnt As Integer

    Public Sub Reset()
        Me.Cnt = 0
    End Sub

    Public Sub Increment()
        Me.Cnt = Me.Cnt + 1
    End Sub

    Public Sub Add(n As Integer)
        Me.Cnt = Me.Cnt + n
    End Sub
End Class

Class Person
    Public Name As String
    Public Age As Integer
    Private _secret As String

    Public Sub Greet()
        Me.Name = "Hello from " + Me.Name
    End Sub

    Public Sub SetSecret(s As String)
        Me._secret = s
    End Sub

    Public Sub GetSecret()
        Me.Name = Me._secret
    End Sub
End Class

' -----------------------------------------------------------------------
' 1. Public field read/write
' -----------------------------------------------------------------------
Print "--- 1. Public field read/write ---"

Dim c As New Counter
c.Cnt = 42
AssertEq "c.Cnt initial", Str$(c.Cnt), "42"

' -----------------------------------------------------------------------
' 2. Method call with no arguments (reads/writes public fields via Me)
' -----------------------------------------------------------------------
Print "--- 2. Method: Reset ---"

c.Reset()
AssertEq "c.Cnt after Reset", Str$(c.Cnt), "0"

' -----------------------------------------------------------------------
' 3. Method that increments internal state
' -----------------------------------------------------------------------
Print "--- 3. Method: Increment ---"

c.Increment()
AssertEq "c.Cnt after Increment", Str$(c.Cnt), "1"
c.Increment()
AssertEq "c.Cnt after 2nd Increment", Str$(c.Cnt), "2"

' -----------------------------------------------------------------------
' 4. Method with integer parameter
' -----------------------------------------------------------------------
Print "--- 4. Method with parameter: Add ---"

c.Add(10)
AssertEq "c.Cnt after Add(10)", Str$(c.Cnt), "12"
c.Add(3)
AssertEq "c.Cnt after Add(3)", Str$(c.Cnt), "15"

' -----------------------------------------------------------------------
' 5. Multiple independent class instances
' -----------------------------------------------------------------------
Print "--- 5. Multiple independent instances ---"

Dim c1 As New Counter
Dim c2 As New Counter
c1.Cnt = 0
c2.Cnt = 0
c1.Add(5)
c2.Add(20)
AssertEq "c1.Cnt independent", Str$(c1.Cnt), "5"
AssertEq "c2.Cnt independent", Str$(c2.Cnt), "20"

' -----------------------------------------------------------------------
' 6. Person class: public string and integer fields
' -----------------------------------------------------------------------
Print "--- 6. Person public fields ---"

Dim p As New Person
p.Name = "Alice"
p.Age = 30
AssertEq "p.Name", p.Name, "Alice"
AssertEq "p.Age", Str$(p.Age), "30"

' -----------------------------------------------------------------------
' 7. Method that writes via Me (Greet modifies Name)
' -----------------------------------------------------------------------
Print "--- 7. Method modifies public field via Me ---"

Dim p2 As New Person
p2.Name = "Bob"
p2.Greet()
AssertEq "p2.Name after Greet", p2.Name, "Hello from Bob"

' -----------------------------------------------------------------------
' 8. Private field: set via SetSecret, read back via GetSecret
' -----------------------------------------------------------------------
Print "--- 8. Private field access through methods ---"

Dim p3 As New Person
p3.Name = "placeholder"
p3.SetSecret("TopSecret")
p3.GetSecret()
AssertEq "private field via getter", p3.Name, "TopSecret"

' -----------------------------------------------------------------------
' 9. Chained calls on same instance
' -----------------------------------------------------------------------
Print "--- 9. Chained calls on same instance ---"

Dim p4 As New Person
p4.Name = "temp"
p4.SetSecret("Alpha")
p4.GetSecret()
AssertEq "first secret", p4.Name, "Alpha"
p4.SetSecret("Beta")
p4.GetSecret()
AssertEq "second secret", p4.Name, "Beta"

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
