' COMMANDS: test_debug_main_local_step.cmds
' EXPECT_OUTPUT: Execution stopped at breakpoint, line 10.|killed:  integer =1|lifeState=|killed=
' Verifies that stepping from a breakpoint inside Modern Main keeps Main's
' local variable scope alive.

Syntax Modern
Function Main(argc As Integer, argv() As String) As Integer
    Dim lifeState As Integer
    Dim killed As Integer
    killed = 1
    lifeState = lifeState + killed
    Print "lifeState=", lifeState, " killed=", killed
    Main = 0
End Function
