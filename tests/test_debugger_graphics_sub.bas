' test_debugger_graphics_sub.bas - stepover inside a graphical subroutine
' SKIP: covered by RunDebuggerGraphicsContract
Syntax Modern

Sub DrawHudLike()
    ScreenLock
    TextOut 8, 8, "debug hud", 15
    ScreenUnlock
End Sub

Function Main(argc As Integer, argv() As String) As Integer
    Cls
    DrawHudLike
    Print "graphics sub ready"
    While 1
        MDelay 50
    Wend

    Main = 0
End Function
