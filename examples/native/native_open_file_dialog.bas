' native_open_file_dialog.bas - select a file with the Windows common dialog
'
' This example uses native DLL calls and native memory helpers. The
' OPENFILENAMEA layout below is for Windows x64.

Syntax Modern
Using runtime
Using String

Declare Function GetOpenFileNameA Lib "comdlg32.dll" _
    Alias "GetOpenFileNameA" _
    (ofn As Pointer) As Bool

Dim ofn As Integer
Dim fileBuffer As Integer
Dim filterBuffer As Integer
Dim titleBuffer As Integer
Dim ok As Integer

Cls

ofn = NativeAlloc(152)
fileBuffer = NativeAlloc(260)
filterBuffer = NativeAlloc(64)
titleBuffer = NativeAlloc(64)

NativePokeStr fileBuffer, 0, "", 260
NativePokeStr filterBuffer, 0, "All files (*.*)" + Chr$(0) + "*.*" + Chr$(0) + Chr$(0), 64
NativePokeStr titleBuffer, 0, "Select a file", 64

' OPENFILENAMEA fields used by this example:
'   lStructSize  offset 0
'   lpstrFilter  offset 24
'   lpstrFile    offset 48
'   nMaxFile     offset 56
'   lpstrTitle   offset 88
'   Flags        offset 96
NativePokeI32 ofn, 0, 152
NativePokePtr ofn, 24, filterBuffer
NativePokePtr ofn, 48, fileBuffer
NativePokeI32 ofn, 56, 260
NativePokePtr ofn, 88, titleBuffer

' OFN_EXPLORER Or OFN_FILEMUSTEXIST Or OFN_PATHMUSTEXIST Or OFN_HIDEREADONLY
NativePokeI32 ofn, 96, 530436

ok = GetOpenFileNameA(ofn)

If ok Then
    Print NativePeekStr$(fileBuffer, 0, 260)
Else
    Print "No file selected"
End If

NativeFree titleBuffer
NativeFree filterBuffer
NativeFree fileBuffer
NativeFree ofn
