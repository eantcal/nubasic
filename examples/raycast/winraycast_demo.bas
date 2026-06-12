Syntax Modern

Using raycast
Using graphics
Using String
Using system

Sub DrawHud(viewW As Integer, viewH As Integer, hudX As Integer)
    FillRect viewW, 0, viewW + 320, viewH, Rgb(16, 18, 22)
    TextOut hudX, 24, "nuBASIC WinRayCast", Rgb(235, 235, 235)
    TextOut hudX, 58, "Sprites: " + Str$(RaySpriteCount()), Rgb(185, 205, 230)
    TextOut hudX, 82, "Actors : " + Str$(RayActorCount()), Rgb(185, 205, 230)
    TextOut hudX, 124, "X " + Str$(RayPlayerX()), Rgb(225, 225, 225)
    TextOut hudX, 148, "Y " + Str$(RayPlayerY()), Rgb(225, 225, 225)
    TextOut hudX, 172, "Facing " + Str$(RayPlayerFacing()), Rgb(225, 225, 225)
    TextOut hudX, 230, "W/S or arrows: move", Rgb(170, 180, 190)
    TextOut hudX, 254, "A/D or arrows: turn", Rgb(170, 180, 190)
    TextOut hudX, 278, "Q/Esc: quit", Rgb(170, 180, 190)
End Sub

Function Main(argc As Integer, argv() As String) As Integer
    Dim viewW As Integer
    Dim viewH As Integer
    Dim hudX As Integer
    Dim moveStep As Integer
    Dim backStep As Integer
    Dim turnStep As Double
    Dim turnLeftStep As Double
    Dim vkEscape As Integer
    Dim vkLeft As Integer
    Dim vkUp As Integer
    Dim vkRight As Integer
    Dim vkDown As Integer
    Dim vkA As Integer
    Dim vkD As Integer
    Dim vkQ As Integer
    Dim vkS As Integer
    Dim vkW As Integer
    Dim scriptBase As String
    Dim world As String
    Dim shouldQuit As Integer
    Dim once As Integer
    Dim ignored As Integer

    viewW = 1024
    viewH = 768
    hudX = 1040
    moveStep = 28
    backStep = 0 - 28
    turnStep = 3.0
    turnLeftStep = 0.0 - 3.0
    vkEscape = 27
    vkLeft = 37
    vkUp = 38
    vkRight = 39
    vkDown = 40
    vkA = 65
    vkD = 68
    vkQ = 81
    vkS = 83
    vkW = 87

    scriptBase = Pwd$()
    If argc > 0 Then
        scriptBase = argv(0)
    End If
    If argc > 1 Then
        If argv(1) = "--once" Then once = 1
    End If

    ignored = RaySetBaseDir(scriptBase)
    world = "raycast_demo/worlds/demo.world.json"

    ignored = RayInit(viewW, viewH)
    If RayLoadProject(world) = 0 Then
        If RayLoadWorld(world) = 0 Then
            Print "Cannot load WinRayCast demo world from: "; scriptBase
            Main = 1
            Exit Function
        End If
    End If

    Screen 1
    MoveWindow 40, 40, viewW + 320, viewH + 40
    ScreenLock

    shouldQuit = 0
    While shouldQuit = 0
        If RayKeyDown(vkEscape) = 1 Or RayKeyDown(vkQ) = 1 Then shouldQuit = 1

        If RayKeyDown(vkLeft) = 1 Or RayKeyDown(vkA) = 1 Then ignored = RayTurn(turnLeftStep)
        If RayKeyDown(vkRight) = 1 Or RayKeyDown(vkD) = 1 Then ignored = RayTurn(turnStep)
        If RayKeyDown(vkUp) = 1 Or RayKeyDown(vkW) = 1 Then ignored = RayMove(moveStep)
        If RayKeyDown(vkDown) = 1 Or RayKeyDown(vkS) = 1 Then ignored = RayMove(backStep)

        ignored = RayUpdate(0.016)
        ignored = RayRender(viewW, viewH)
        ignored = RayPresent(0, 0, viewW, viewH)
        DrawHud viewW, viewH, hudX
        Refresh
        If once = 1 Then shouldQuit = 1
        MDelay(16)
    Wend

    ScreenUnlock
    Main = 0
End Function
