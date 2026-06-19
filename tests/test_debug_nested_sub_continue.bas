' test_debug_nested_sub_continue.bas - continue from nested Sub breakpoint
' COMMANDS: test_debug_nested_sub_continue.commands
' EXPECT_OUTPUT: Execution stopped at breakpoint, line 14|lifeState=	2|killed=	1
Syntax Modern

Class EventLog
    Public Line1 As String
End Class

Sub DrawMiniMapLike(mapUnlocks As Integer)
    Dim radius As Integer
    Dim tile As Integer
    Dim gridSize As Integer
    radius = 5
    tile = 14
    gridSize = (radius * 2 + 1) * tile
    Print "inside map"
End Sub

Sub DrawHudLike(viewW As Integer, _
    viewH As Integer, _
    hudX As Integer, _
    mapUnlocks As Integer, _
    eventLog As EventLog)

    DrawMiniMapLike mapUnlocks
End Sub

Function Main(argc As Integer, argv() As String) As Integer
    Dim lifeState As Integer
    Dim killed As Integer
    Dim viewW As Integer
    Dim viewH As Integer
    Dim hudX As Integer
    Dim mapUnlocks As Integer
    Dim eventLog As EventLog

    lifeState = 1
    killed = 1
    viewW = 1024
    viewH = 1024
    hudX = 1040
    mapUnlocks = 0
    eventLog = New EventLog()
    eventLog.Line1 = "ready"

    DrawHudLike viewW, _
        viewH, _
        hudX, _
        mapUnlocks, _
        eventLog

    lifeState = lifeState + killed
    Print "lifeState=", lifeState, " killed=", killed

    Main = 0
End Function
