' test_native_uint_range.bas - native unsigned argument range validation
' EXPECT_ERROR: value is out of range for DWORD

Syntax Modern

Declare Function WinBeep Lib "kernel32.dll" Alias "Beep" (freq As DWORD, duration As DWORD) As Bool
Print WinBeep(-1, 1)
