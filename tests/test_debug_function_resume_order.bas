' COMMANDS: test_debug_function_resume_order.cmds
' EXPECT_OUTPUT: before|entry 1|Execution stopped at breakpoint, line 9.|after break|done|final=1 calls=1
' Verifies that continuing from a breakpoint inside an expression-called
' function resumes the suspended function instead of re-running it from entry.

Dim calls% As Integer
Function F%()
    calls% = calls% + 1
    Print "entry " + Str$(calls%)
    Print "after break"
    F% = calls%
End Function

Dim result% As Integer
Print "before"
result% = F%()
Print "done"
Print "final=" + Str$(result%) + " calls=" + Str$(calls%)
