' EXPECT_ERROR: cannot access private member

Syntax Modern

Class Vault
    Private Static Function Hidden() As Integer
        Hidden = 42
    End Function
End Class

Print Vault.Hidden()
