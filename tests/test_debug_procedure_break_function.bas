' COMMANDS: test_debug_procedure_break_function.cmds
' EXPECT_OUTPUT: before function|Execution stopped at breakpoint, line 9.
' Verifies that a breakpoint on the first line inside a Function body fires on call entry.

Dim y% As Integer
y% = 0

Function Twice%(n% As Integer)
    Print "inside function"
    Twice% = n% * 2
End Function

Print "before function"
Call Twice%(3)
Print "after function"
