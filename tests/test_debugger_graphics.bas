' test_debugger_graphics.bas - graphics debugger contract fixture
' SKIP: covered by RunDebuggerGraphicsContract
Cls
FillRect 0, 0, 80, 80, 0
ScreenLock
Line 0, 0, 40, 40, 15
ScreenUnlock
Print "graphics ready"
While 1
    MDelay 50
Wend
