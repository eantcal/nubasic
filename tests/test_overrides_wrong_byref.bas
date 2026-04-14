' EXPECT_ERROR: ByRef

Syntax Modern

Class Base
    Overridable Sub Foo(ByRef x% As Integer)
    End Sub
End Class

Class Child
    Inherits Base
    Overrides Sub Foo(x% As Integer)
    End Sub
End Class
