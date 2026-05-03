' test_native_declare.bas - native DLL declarations
' EXPECT_OUTPUT: native declarations parsed|5

Syntax Modern

Declare Function GetCurrentProcessId Lib "kernel32.dll" () As DWORD
Declare Function lstrlenA Lib "kernel32.dll" Alias "lstrlenA" (text As String) As Integer

Cls

Print "native declarations parsed"
Print lstrlenA("hello")