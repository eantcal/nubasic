' native_dll_function.bas - call a simple function exported by a Windows DLL

Syntax Modern
Using runtime

Declare Function lstrlenA Lib "kernel32.dll" _
    Alias "lstrlenA" _
    (text As String) As Integer

Declare Function GetCurrentProcessId Lib "kernel32.dll" _
    Alias "GetCurrentProcessId" _
    () As DWORD

Print "Length of nuBASIC: "; lstrlenA("nuBASIC")
Print "Current process id: "; GetCurrentProcessId()
