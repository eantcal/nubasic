' EXPECT_ERROR: ByRef

Class Base
    Overridable Sub Foo(ByRef x% As Integer)
    End Sub
End Class

Class Child
    Inherits Base
    Overrides Sub Foo(x% As Integer)
    End Sub
End Class
