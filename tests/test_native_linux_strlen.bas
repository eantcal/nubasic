' test_native_linux_strlen.bas - call strlen() from glibc on Linux
' PLATFORM: linux
' EXPECT_OUTPUT: native call ok|5

Syntax Modern

Declare Function strlen Lib "libc.so.6" (text As String) As Integer

Print "native call ok"
Print strlen("hello")
