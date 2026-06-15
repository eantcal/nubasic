' test_main_entry_case.bas
' EXPECT_OUTPUT: uppercase Main entry invoked|argc=2|argv1=case-test
' ARGS: case-test

Syntax Modern
Using string

Function Main(argc As Integer, argv() As String) As Integer
    Print "uppercase Main entry invoked"
    Print "argc=" + Str$(argc)
    Print "argv1=" + argv(1)
    Main = 0
End Function
