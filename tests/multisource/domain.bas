' Domain model included by test_multisource.bas.
' Demonstrates a class defined in a separate source file.

Syntax Modern
Using String

Include "nested/config.bas"

' A named accumulator that tracks a running total and a step count.
Class MultiCounter
    Public Name$ As String
    Private Value% As Integer
    Private Steps% As Integer

    Public Sub New(name As String, initial As Integer)
        Me.Name$ = name
        Me.Value% = initial
        Me.Steps% = 0
    End Sub

    Public Sub Add(delta As Integer)
        Me.Value% = Me.Value% + delta
        Me.Steps% = Me.Steps% + 1
    End Sub

    Public Sub Reset()
        Me.Value% = 0
        Me.Steps% = 0
    End Sub

    Public Function GetValue%()
        GetValue% = Me.Value%
    End Function

    Public Function GetSteps%()
        GetSteps% = Me.Steps%
    End Function

    ' Returns "name=value"
    Public Function Report$()
        Report$ = Me.Name$ + "=" + Str$(Me.Value%)
    End Function
End Class