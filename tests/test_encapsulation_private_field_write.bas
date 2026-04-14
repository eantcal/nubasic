' EXPECT_ERROR: cannot access private member

Syntax Modern

Class Vault
    Private Secret As String
End Class

Dim v As New Vault()
v.Secret = "alpha"
