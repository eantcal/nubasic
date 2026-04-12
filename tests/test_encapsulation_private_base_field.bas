' EXPECT_ERROR: cannot access private member

Class BaseVault
    Private Secret As String
End Class

Class ChildVault
    Inherits BaseVault

    Public Sub Leak()
        Me.Secret = "x"
    End Sub
End Class

Dim c As New ChildVault()
c.Leak()
