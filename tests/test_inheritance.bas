' test_inheritance.bas
' Tests for VB-style single inheritance (Phase 2)
'
' Tests:
'   1. Derived class inherits public field from base
'   2. Derived class inherits public method from base (not overridden)
'   3. Derived class overrides a method (Overrides / Overridable)
'   4. Derived class adds its own method
'   5. Base class method still works independently
'   6. Multi-level inheritance (3 levels: A -> B -> C)
'   7. Derived field declared in derived class works alongside inherited field

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
' Class hierarchy
' -----------------------------------------------------------------------

Class Animal
    Public Name As String

    Public Overridable Sub Speak()
        Me.Name = Me.Name + " says ..."
    End Sub

    Public Sub Describe()
        Me.Name = "Animal:" + Me.Name
    End Sub
End Class

Class Dog
    Inherits Animal

    Public Breed As String

    Public Overrides Sub Speak()
        Me.Name = Me.Name + " says Woof!"
    End Sub

    Public Sub Fetch()
        Me.Name = Me.Name + " fetches"
    End Sub
End Class

Class Puppy
    Inherits Dog

    Public Overrides Sub Speak()
        Me.Name = Me.Name + " says Yip!"
    End Sub
End Class

' -----------------------------------------------------------------------
' 1. Inherited public field
' -----------------------------------------------------------------------
Print "--- 1. Inherited public field ---"

Dim d As New Dog
d.Name = "Rex"
AssertEq "d.Name (inherited field)", d.Name, "Rex"

' -----------------------------------------------------------------------
' 2. Inherited method (not overridden: Describe)
' -----------------------------------------------------------------------
Print "--- 2. Inherited method ---"

Dim d2 As New Dog
d2.Name = "Max"
d2.Describe()
AssertEq "d2.Name after Describe", d2.Name, "Animal:Max"

' -----------------------------------------------------------------------
' 3. Overridden method: Dog.Speak overrides Animal.Speak
' -----------------------------------------------------------------------
Print "--- 3. Overridden method ---"

Dim d3 As New Dog
d3.Name = "Buddy"
d3.Speak()
AssertEq "d3.Name after Dog.Speak", d3.Name, "Buddy says Woof!"

' -----------------------------------------------------------------------
' 4. Derived class own method
' -----------------------------------------------------------------------
Print "--- 4. Derived own method ---"

Dim d4 As New Dog
d4.Name = "Luna"
d4.Fetch()
AssertEq "d4.Name after Fetch", d4.Name, "Luna fetches"

' -----------------------------------------------------------------------
' 5. Base class works independently
' -----------------------------------------------------------------------
Print "--- 5. Base class unaffected ---"

Dim a As New Animal
a.Name = "Cat"
a.Speak()
AssertEq "a.Name after Animal.Speak", a.Name, "Cat says ..."

' -----------------------------------------------------------------------
' 6. Multi-level: Puppy inherits from Dog inherits from Animal
' -----------------------------------------------------------------------
Print "--- 6. Multi-level inheritance ---"

Dim p As New Puppy
p.Name = "Spot"
p.Speak()
AssertEq "p.Speak (Puppy override)", p.Name, "Spot says Yip!"

Dim p2 As New Puppy
p2.Name = "Fido"
p2.Describe()
AssertEq "p2.Describe (inherited from Animal)", p2.Name, "Animal:Fido"

Dim p3 As New Puppy
p3.Name = "Tiny"
p3.Fetch()
AssertEq "p3.Fetch (inherited from Dog)", p3.Name, "Tiny fetches"

' -----------------------------------------------------------------------
' 7. Derived field alongside inherited field
' -----------------------------------------------------------------------
Print "--- 7. Derived own field alongside inherited field ---"

Dim d5 As New Dog
d5.Name = "Ace"
d5.Breed = "Labrador"
AssertEq "d5.Name", d5.Name, "Ace"
AssertEq "d5.Breed", d5.Breed, "Labrador"

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
