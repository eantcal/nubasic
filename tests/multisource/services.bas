' Service source included by test_multisource.bas.

Sub AddTwice(counter As MultiCounter, delta As Integer)
    counter.Add(delta)
    counter.Add(delta)
End Sub

Function ScaleValue%(value As Integer)
    ScaleValue% = value * Multiplier%()
End Function

Function ServiceSummary$(counter As MultiCounter)
    ServiceSummary$ = "service:" + counter.Report$()
End Function
