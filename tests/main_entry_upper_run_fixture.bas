Syntax Modern
Using string

Function Main(argc As Integer, argv() As String) As Integer
    Print "uppercase Main run invoked"
    Print "argc=" + Str$(argc)
    Main = 0
End Function
