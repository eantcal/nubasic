' EXPECT_ERROR: cannot access private member

Class Vault
    Private Secret As String

    Public Sub SetSecret(s As String)
        Me.Secret = s
    End Sub
End Class

Dim v As New Vault()
v.SetSecret("alpha")
Print v.Secret
