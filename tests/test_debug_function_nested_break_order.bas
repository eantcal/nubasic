' COMMANDS: test_debug_function_nested_break_order.cmds
' EXPECT_OUTPUT: Execution stopped at breakpoint, line 17.|Call stack:|f3 called from line 12|f2 called from line 7|f1 called from line 20|value=1
' EXPECT_NOT_OUTPUT: value=0|Runtime Error|missing function return value
Syntax Modern

Function f1() As Integer
    Call f2()
    f1 = 1
End Function

Function f2() As Integer
    Call f3()
    f2 = 2
End Function

Function f3() As Integer
    f3 = 3
End Function

Print "value=" + string::str$(f1())
End
