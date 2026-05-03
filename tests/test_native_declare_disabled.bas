' test_native_declare_disabled.bas - native DLL calls can be explicitly disabled
' PLATFORM: windows
' ARGS: --disable-native-calls
' EXPECT_ERROR: Native DLL calls are disabled by --disable-native-calls.

Syntax Modern

Declare Function GetCurrentProcessId Lib "kernel32.dll" () As DWORD
Print GetCurrentProcessId()
