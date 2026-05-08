' test_class_array.bas
' Tests for arrays of class instances (Dim arr(N) As ClassName).
' Mirrors test_struct_array.bas for class types.
'
' This file covers what currently works after the fix that lifted
' the early "type illegal" guard for class arrays:
'   1. Top-level class array + per-slot New
'   2. Field read on indexed slot: arr(i).field
'   3. Field write on indexed slot: arr(i).field = value
'   4. Loop fill with variable index
'   5. Object reference (alias) semantics on a slot
'
' Known limitations not covered here yet (see issue tracker):
'   - method dispatch on indexed slot (m(i).Method()) currently always
'     reads slot 0
'   - polymorphic dispatch on base-typed array (shapes(i).Override())
'     does not call the derived override
'   - assigning to a class-array field of Me from inside a method, in a
'     loop, fails on the second iteration
'   - "obj.field(i) = New X()" from outside the class triggers a parse
'     error in some forms

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

' -----------------------------------------------------------------------
Class Box
    Public value% As Integer

    Sub New()
        Me.value% = 0
    End Sub
End Class

' -----------------------------------------------------------------------
' 1. Per-slot New on top-level class array
' -----------------------------------------------------------------------
Print "--- 1. Per-slot New ---"

Dim arr(4) As Box
arr(0) = New Box()
arr(1) = New Box()
arr(2) = New Box()
arr(3) = New Box()
arr(0).value% = 11
arr(1).value% = 22
arr(2).value% = 33
arr(3).value% = 44
AssertEq "arr(0).value%", Str$(arr(0).value%), "11"
AssertEq "arr(1).value%", Str$(arr(1).value%), "22"
AssertEq "arr(2).value%", Str$(arr(2).value%), "33"
AssertEq "arr(3).value%", Str$(arr(3).value%), "44"

' -----------------------------------------------------------------------
' 2. Loop fill with variable index
' -----------------------------------------------------------------------
Print "--- 2. Loop fill of class array ---"

Dim arr2(4) As Box
Dim i% As Integer
For i% = 0 To 3
    arr2(i%) = New Box()
    arr2(i%).value% = i% * 10
Next i%
For i% = 0 To 3
    AssertEq "arr2(" + Str$(i%) + ").value%", Str$(arr2(i%).value%), Str$(i% * 10)
Next i%

' -----------------------------------------------------------------------
' 3. Object reference (alias) semantics across slots
' -----------------------------------------------------------------------
Print "--- 3. Alias semantics on class-array slot ---"

Dim a(2) As Box
a(0) = New Box()
a(0).value% = 1
Dim ref As Box
ref = a(0)
ref.value% = 99
AssertEq "alias mutation visible through a(0)", Str$(a(0).value%), "99"

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
