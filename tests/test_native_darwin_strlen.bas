' test_native_darwin_strlen.bas - call strlen() from libSystem on macOS
' PLATFORM: darwin
' EXPECT_OUTPUT: native call ok|5

Syntax Modern

Declare Function strlen Lib "libSystem.B.dylib" (text As String) As Integer

Print "native call ok"
Print strlen("hello")
