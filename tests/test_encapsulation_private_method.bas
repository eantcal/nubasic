' EXPECT_ERROR: cannot access private member

Syntax Modern

Class Vault
    Private Sub Hide()
        Print "hidden"
    End Sub
End Class

Dim v As New Vault()
v.Hide()
