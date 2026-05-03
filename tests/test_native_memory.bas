' test_native_memory.bas - native memory helpers for pointer arguments
' EXPECT_OUTPUT: native memory ok|5|hello

Syntax Modern
Using runtime

Declare Function lstrlenA Lib "kernel32.dll" Alias "lstrlenA" (text As Pointer) As Integer

Dim p As Integer
p = NativeAlloc(32)
NativePokeStr p, 0, "hello", 32

Print "native memory ok"
Print lstrlenA(p)
Print NativePeekStr$(p, 0, 32)

NativeFree p
