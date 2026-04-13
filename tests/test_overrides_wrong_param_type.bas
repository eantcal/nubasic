' EXPECT_ERROR: type

Class Base
    Overridable Sub Foo(x% As Integer)
    End Sub
End Class

Class Child
    Inherits Base
    Overrides Sub Foo(x$)
    End Sub
End Class
