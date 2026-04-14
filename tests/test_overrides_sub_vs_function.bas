' EXPECT_ERROR: cannot override

Syntax Modern

Class Base
    Overridable Sub Foo()
    End Sub
End Class

Class Child
    Inherits Base
    Overrides Function Foo()
        Foo = 0
    End Function
End Class
