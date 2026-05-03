' test_native_wrong_arg_count.bas - native invocation validates arity
' EXPECT_ERROR: Runtime Error

Syntax Modern

Declare Function lstrlenA Lib "kernel32.dll" Alias "lstrlenA" (text As String) As Integer
Print lstrlenA()
