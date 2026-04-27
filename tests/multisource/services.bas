' Service layer included by test_multisource.bas via #Include.
' Defines helper routines that operate on MultiCounter objects from domain.bas.

Syntax Modern
Using String

Include "nested/config.bas"

Sub AddTwice(counter As MultiCounter, delta As Integer)
    counter.Add(delta)
    counter.Add(delta)
End Sub

Function ServiceSummary$(counter As MultiCounter)
    ServiceSummary$ = "service:" + counter.Report$()
End Function

Function StatusLine$(counter As MultiCounter)
    Dim tag As String
    Dim stateStr As String
    Dim steps As String
    tag = SourceTag$()
    stateStr = counter.Name$ + "=" + Str$(counter.GetValue%())
    steps = Str$(counter.GetSteps%())
    StatusLine$ = "[" + tag + "] " + stateStr + " steps=" + steps
End Function
