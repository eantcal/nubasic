' test_ctor.bas — parameterized constructors

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

' --- 1. Basic constructor with two integer args ---

Class Point
    Public x As Integer
    Public y As Integer

    Sub New(px As Integer, py As Integer)
        Me.x = px
        Me.y = py
    End Sub
End Class

Print "--- 1. Basic constructor ---"
Dim p As New Point(3, 7)
AssertEq "p.x", Str$(p.x), "3"
AssertEq "p.y", Str$(p.y), "7"

' --- 2. Constructor with string arg ---

Class Greeter
    Public name$ As String

    Sub New(n$ As String)
        Me.name$ = n$
    End Sub

    Function Greet$()
        Greet$ = "Hello, " + Me.name$
    End Function
End Class

Print "--- 2. String arg + method ---"
Dim g As New Greeter("World")
AssertEq "g.Greet$()", g.Greet$(), "Hello, World"

' --- 3. No-arg constructor (Sub New with no params) ---

Class Counter
    Public value% As Integer

    Sub New()
        Me.value% = 100
    End Sub
End Class

Print "--- 3. No-arg constructor ---"
Dim c As New Counter()
AssertEq "c.value%", Str$(c.value%), "100"

' --- 4. Inherited constructor (derived class has own Sub New) ---

Class Animal
    Public name$ As String
End Class

Class Dog
    Inherits Animal
    Public breed$ As String

    Sub New(n$ As String, b$ As String)
        Me.name$ = n$
        Me.breed$ = b$
    End Sub
End Class

Print "--- 4. Inherited constructor ---"
Dim d As New Dog("Rex", "Labrador")
AssertEq "d.name$",  d.name$,  "Rex"
AssertEq "d.breed$", d.breed$, "Labrador"

' --- Results ---

Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
