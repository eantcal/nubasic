' COMMANDS: test_debug_procedure_break_sub.cmds
' EXPECT_OUTPUT: before sub|Execution stopped at breakpoint, line 9.
' Verifies that a breakpoint on the first executable line inside a Sub body fires on call entry.

Dim x% As Integer
x% = 0

Sub Touch()
    x% = x% + 1
End Sub

Print "before sub"
Call Touch()
Print "after sub"
