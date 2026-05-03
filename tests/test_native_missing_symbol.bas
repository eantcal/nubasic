' test_native_missing_symbol.bas - native symbol resolution error
' PLATFORM: windows
' EXPECT_ERROR: Cannot resolve native symbol

Syntax Modern

Declare Function MissingKernelExport Lib "kernel32.dll" Alias "DefinitelyMissingKernelExport" () As DWORD
Print MissingKernelExport()
