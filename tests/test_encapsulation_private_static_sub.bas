' EXPECT_ERROR: cannot access private member

Class Vault
    Private Static Sub Hide()
        Print "hidden"
    End Sub
End Class

Vault.Hide()
