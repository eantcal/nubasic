' EXPECT_ERROR: cannot access private member

Syntax Modern

Class Vault
    Private Static Sub Hide()
        Print "hidden"
    End Sub
End Class

Vault.Hide()
