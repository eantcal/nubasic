' EXPECT_ERROR: cannot access private member

Syntax Modern

Class Hidden
    Private Sub New()
    End Sub
End Class

Dim h As Hidden
h = New Hidden()
