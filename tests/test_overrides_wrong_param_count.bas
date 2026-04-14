' EXPECT_ERROR: parameter count

Syntax Modern

Class Base
    Overridable Sub Foo(x% As Integer)
    End Sub
End Class

Class Child
    Inherits Base
    Overrides Sub Foo(x% As Integer, y% As Integer)
    End Sub
End Class
