' main.bas - multi-source program with a main() entry point

Syntax Modern
Using String

Include "lib/messages.bas"
Include "lib/calculator.bas"

Function main() As Integer
    Dim name As String
    name = "nuBASIC"

    Print BuildGreeting$(name)
    Print "2 + 3 = "; Add%(2, 3)
    Print "2 * 3 * 4 = "; Product3%(2, 3, 4)
    Print RepeatText$("-", 24)
    Print "main() returned success"

    main = 0
End Function
