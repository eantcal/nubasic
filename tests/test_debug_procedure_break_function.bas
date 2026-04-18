' COMMANDS: test_debug_procedure_break_function.cmds
' EXPECT_OUTPUT: before function|Execution stopped at breakpoint, line 8.
' Verifies procedure-entry breakpoints for Function bodies invoked by Call.

Dim y% As Integer
y% = 0

Function Twice%(n% As Integer)
    Print "inside function"
    Twice% = n% * 2
End Function

Print "before function"
Call Twice%(3)
Print "after function"
