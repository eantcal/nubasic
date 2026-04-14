' EXPECT_ERROR: no Overridable method

Syntax Modern

Class Base
    Sub Foo()
    End Sub
End Class

Class Child
    Inherits Base
    Overrides Sub Foo()
    End Sub
End Class
