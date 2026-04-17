' Domain source included by test_multisource.bas.

Include "nested/config.bas"

Class MultiCounter
    Public Name$ As String
    Private Value% As Integer

    Public Sub New(name As String, initial As Integer)
        Me.Name$ = name
        Me.Value% = initial
    End Sub

    Public Sub Add(delta As Integer)
        Me.Value% = Me.Value% + delta
    End Sub

    Public Function Report$()
        Report$ = Me.Name$ + "=" + Str$(Me.Value%)
    End Function
End Class
