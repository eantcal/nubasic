' test_native_unknown_type.bas - declaration rejects unknown native types
' EXPECT_ERROR: Unknown native parameter type

Syntax Modern

Declare Function BadNative Lib "kernel32.dll" (x As BogusType) As Integer
