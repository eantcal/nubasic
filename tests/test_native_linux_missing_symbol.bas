' test_native_linux_missing_symbol.bas - dlsym() failure for missing export
' PLATFORM: linux
' EXPECT_ERROR: Cannot resolve native symbol

Syntax Modern

Declare Function definitely_missing_libc_export Lib "libc.so.6" () As Integer

Print definitely_missing_libc_export()
