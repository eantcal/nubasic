' EXPECT_ERROR: null object reference

Class Box
    Public Value% As Integer

    Public Sub SetValue(n% As Integer)
        Me.Value% = n%
    End Sub
End Class

Dim b As Box
b.SetValue(1)
