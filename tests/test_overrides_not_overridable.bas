' EXPECT_ERROR: no Overridable method

Class Base
    Sub Foo()
    End Sub
End Class

Class Child
    Inherits Base
    Overrides Sub Foo()
    End Sub
End Class
