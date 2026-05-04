' messages.bas - string helpers included by ../main.bas

Syntax Modern
Using String

Include "config.bas"

Function BuildGreeting$(name As String)
    BuildGreeting$ = AppTitle$() + ": hello, " + name
End Function

Function RepeatText$(text As String, count As Integer)
    Dim i% As Integer
    RepeatText$ = ""

    For i% = 1 To count
        RepeatText$ = RepeatText$ + text
    Next i%
End Function
