' test_destructor.bas - class destructors

Syntax Modern
Using string

Dim pass% As Integer
Dim fail% As Integer
Dim log$ As String
pass% = 0
fail% = 0
log$ = ""

Sub AssertEq(label$ As String, got$ As String, expected$ As String)
    If got$ = expected$ Then
        Print "  PASS  " + label$
        pass% = pass% + 1
    Else
        Print "  FAIL  " + label$ + "  got='" + got$ + "'  expected='" + expected$ + "'"
        fail% = fail% + 1
    End If
End Sub

Class Resource
    Public name$ As String

    Sub New(n$ As String)
        Me.name$ = n$
    End Sub

    Sub Delete()
        log$ = log$ + "delete:" + Me.name$ + ";"
    End Sub
End Class

Class BaseGuard
    Sub Delete()
        log$ = log$ + "base;"
    End Sub
End Class

Class DerivedGuard
    Inherits BaseGuard

    Sub Delete()
        log$ = log$ + "derived;"
    End Sub
End Class

Sub MakeResources()
    Dim first As New Resource("first")
    Dim second As New Resource("second")
End Sub

Sub MakeDerived()
    Dim guard As New DerivedGuard()
End Sub

Sub BorrowResource(value As Resource)
End Sub

Print "--- 1. Local destructors run in reverse definition order ---"
MakeResources()
AssertEq "local destructor order", log$, "delete:second;delete:first;"

Print "--- 2. Derived destructor runs before base destructor ---"
log$ = ""
MakeDerived()
AssertEq "inheritance destructor order", log$, "derived;base;"

Print "--- 3. Borrowed object is not destroyed by callee scope ---"
log$ = ""
Dim globalResource As New Resource("global")
BorrowResource(globalResource)
AssertEq "borrowed parameter", log$, ""

Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
