Syntax Modern

' WinRayCast demo in nuBASIC.
'
' This file is intentionally written as a tutorial.
' The GAME SETTINGS section contains the parameters that are meant to be changed,
' while the main loop shows how a nuBASIC program can drive the WinRayCast engine.
'
' WinRayCast provides the pseudo-3D raycasting engine.
' nuBASIC provides the scripting layer, graphics window, HUD drawing and game logic.

Using raycast
Using graphics
Using String
Using system

' Calculates an integer percentage used by the Doom-style HUD.
Function Percent(done As Integer, total As Integer) As Integer
    If total <= 0 Then
        Percent = 100
    Else
        Percent = (done * 100) / total
    End If
End Function

' Draws a percentage meter, for example the player energy bar.
Sub DrawMeter(x As Integer, y As Integer, w As Integer, h As Integer, value As Integer, color As Integer)
    Dim fillW As Integer

    ' Clamp the value to the valid percentage range.
    If value < 0 Then value = 0
    If value > 100 Then value = 100

    fillW = (w * value) / 100

    ' Background of the meter.
    FillRect x, y, x + w, y + h, Rgb(45, 48, 56)

    ' Filled portion of the meter.
    FillRect x, y, x + fillW, y + h, color

    ' Border.
    Rect x, y, x + w, y + h, Rgb(155, 160, 170)
End Sub

' Draws a progressive red overlay during death and respawn.
Sub DrawDeathOverlay(viewW As Integer, viewH As Integer, strength As Integer, respawning As Integer)
    Dim edgeSize As Integer
    Dim color As Integer

    If strength <= 0 Then Exit Sub
    If strength > 100 Then strength = 100

    ' The stronger the effect, the thicker the red border becomes.
    edgeSize = (viewH * strength) / 220
    color = Rgb(130 + (strength / 2), 0, 0)

    ' Draw four red borders around the 3D viewport.
    FillRect 0, 0, viewW, edgeSize, color
    FillRect 0, viewH - edgeSize, viewW, viewH, color
    FillRect 0, 0, edgeSize, viewH, color
    FillRect viewW - edgeSize, 0, viewW, viewH, color

    If respawning = 1 Then
        TextOut 360, 350, "RESTORING LAST AUTOSAVE", Rgb(255, 230, 230)
    Else
        TextOut 438, 350, "ENERGY DEPLETED", Rgb(255, 230, 230)
    End If
End Sub

' Draws the side HUD panel.
'
' The HUD is mostly BASIC-side logic.
' It queries WinRayCast through Ray* functions to display engine state such as:
' - number of sprites
' - number of actors
' - number of enemies
' - killed enemies
' - collected items
' - player position and facing direction
Sub DrawHud(viewW As Integer, viewH As Integer, hudX As Integer, energy As Integer, autosaveTimer As Double, autosaveSeconds As Double, saved As Integer, gameDone As Integer)
    Dim enemies As Integer
    Dim killed As Integer
    Dim killPct As Integer
    Dim itemPct As Integer
    Dim items As Integer
    Dim collected As Integer
    Dim autoRemain As Integer

    ' Query the current game state from the raycast engine.
    enemies = RayEnemyCount()
    killed = RayKilledEnemyCount()
    items = RayItemCount()
    collected = RayCollectedItemCount()

    killPct = Percent(killed, enemies)
    itemPct = Percent(collected, items)

    ' HUD background.
    FillRect viewW, 0, viewW + 320, viewH, Rgb(16, 18, 22)

    TextOut hudX, 24, "nuBASIC WinRayCast", Rgb(235, 235, 235)

    ' Debug/engine information.
    TextOut hudX, 58, "Sprites: " + Str$(RaySpriteCount()), Rgb(185, 205, 230)
    TextOut hudX, 82, "Actors : " + Str$(RayActorCount()), Rgb(185, 205, 230)

    ' Player energy.
    TextOut hudX, 124, "ENERGY " + Str$(energy) + "%", Rgb(225, 225, 225)
    DrawMeter hudX, 148, 220, 16, energy, Rgb(60, 190, 115)

    ' Progress counters.
    TextOut hudX, 188, "KILLS " + Str$(killed) + "/" + Str$(enemies) + " " + Str$(killPct) + "%", Rgb(230, 230, 230)
    TextOut hudX, 212, "ITEMS " + Str$(collected) + "/" + Str$(items) + " " + Str$(itemPct) + "%", Rgb(230, 230, 230)

    ' Player position and facing direction.
    TextOut hudX, 254, "X " + Str$(RayPlayerX()), Rgb(225, 225, 225)
    TextOut hudX, 278, "Y " + Str$(RayPlayerY()), Rgb(225, 225, 225)
    TextOut hudX, 302, "Facing " + Str$(RayPlayerFacing()), Rgb(225, 225, 225)

    ' Autosave status.
    If saved = 1 Then
        autoRemain = autosaveSeconds - autosaveTimer
        If autoRemain < 0 Then autoRemain = 0
        TextOut hudX, 344, "Autosave in " + Str$(autoRemain) + "s", Rgb(170, 210, 255)
    Else
        TextOut hudX, 344, "Autosave pending", Rgb(170, 210, 255)
    End If

    ' Mission status.
    If gameDone = 1 Then
        TextOut hudX, 392, "MISSION COMPLETE", Rgb(150, 230, 150)
    Else
        TextOut hudX, 392, "Goal: kill 100% enemies", Rgb(190, 195, 205)
    End If

    ' Controls.
    TextOut hudX, 454, "W/S or arrows: move", Rgb(170, 180, 190)
    TextOut hudX, 478, "A/D or arrows: turn", Rgb(170, 180, 190)
    TextOut hudX, 502, "Space/Ctrl: fire", Rgb(170, 180, 190)
    TextOut hudX, 526, "Q/Esc: quit", Rgb(170, 180, 190)
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
    Dim vkSpace As Integer
    Dim vkCtrl As Integer
    Dim scriptBase As String
    Dim world As String
    Dim shouldQuit As Integer
    Dim once As Integer
    Dim energy As Double
    Dim energyInt As Integer
    Dim delta As Double
    Dim moving As Integer
    Dim autosaveTimer As Double
    Dim checkpointSaved As Integer
    Dim checkpointX As Integer
    Dim checkpointY As Integer
    Dim checkpointFacing As Double
    Dim lifeState As Integer
    Dim deathTimer As Double
    Dim redStrength As Integer
    Dim gameDone As Integer
    Dim enemies As Integer
    Dim killed As Integer
    Dim shotCooldown As Double
    Dim incomingDamage As Double
    Dim autosaveSeconds As Double
    Dim respawnSeconds As Double
    Dim deathOverlaySeconds As Double
    Dim energyMoveCost As Double
    Dim energyShotCost As Double
    Dim fireDamage As Double
    Dim fireRangeCells As Double
    Dim fireFovDegrees As Double
    Dim fireCooldownSeconds As Double

    ' ------------------------------------------------------------------
    ' GAME SETTINGS
    '
    ' Change these values to alter the behaviour of the demo without
    ' touching the rest of the program.
    ' ------------------------------------------------------------------

    viewW = 1024               ' 3D viewport width, aligned with the C++ demo.
    viewH = 1024               ' 3D viewport height; square viewport avoids 4:3 distortion.
    hudX = viewW + 16          ' Initial X position of the side HUD panel.

    delta = 0.016              ' Simulated frame delta, roughly 60 FPS.
    ' Used by animations, doors, enemies and timers.

    moveStep = 28              ' Forward movement step per frame.
    backStep = 0 - moveStep    ' Negative movement step for walking backwards.

    turnStep = 3.0             ' Rotation step in degrees per frame.
    turnLeftStep = 0.0 - turnStep

    energy = 100.0             ' Initial player energy.

    energyMoveCost = 0.08      ' Energy cost per frame while moving.
    energyShotCost = 0.25      ' Energy cost per shot.

    autosaveSeconds = 5.0      ' Time between automatic checkpoints.
    respawnSeconds = 3.25      ' Delay before restoring from the last autosave.
    deathOverlaySeconds = 2.0  ' Time needed for the red overlay to reach full strength.

    fireDamage = 35.0          ' Base damage dealt by the player shot.
    fireRangeCells = 12.0      ' Maximum shooting range, expressed in map cells.
    fireFovDegrees = 28.0      ' Front cone angle used by the shot.
    fireCooldownSeconds = 0.35 ' Minimum time between two shots.

    ' ------------------------------------------------------------------
    ' WINDOWS VIRTUAL KEYS
    '
    ' RayKeyDown() uses Windows virtual-key codes.
    ' This keeps the BASIC demo simple and avoids implementing a custom
    ' keyboard input layer.
    ' ------------------------------------------------------------------

    vkEscape = 27              ' Esc: quit.
    vkLeft = 37                ' Left arrow: turn left.
    vkUp = 38                  ' Up arrow: move forward.
    vkRight = 39               ' Right arrow: turn right.
    vkDown = 40                ' Down arrow: move backward.
    vkA = 65                   ' A: turn left.
    vkD = 68                   ' D: turn right.
    vkQ = 81                   ' Q: quit.
    vkS = 83                   ' S: move backward.
    vkW = 87                   ' W: move forward.
    vkSpace = 32               ' Space: fire.
    vkCtrl = 17                ' Ctrl: fire.

    ' Force the first autosave shortly after loading the world.
    autosaveTimer = autosaveSeconds

    ' By default, use the current working directory as base path.
    scriptBase = Pwd$()

    ' If nuBASIC passes the script path as argv(0), use it as the base path.
    If argc > 0 Then
        scriptBase = argv(0)
    End If

    ' Optional test mode: render only one frame and exit.
    If argc > 1 Then
        If argv(1) = "--once" Then once = 1
    End If

    ' Tell WinRayCast where relative resources should be resolved from.
    ' This makes the demo independent from the IDE current working directory.
    RaySetBaseDir(scriptBase)

    ' WinRayCast world/project file used by the demo.
    world = "raycast_demo/worlds/demo.world.json"

    ' ------------------------------------------------------------------
    ' ENGINE INITIALIZATION
    ' ------------------------------------------------------------------

    ' Initialize a WinRayCast session with the selected software framebuffer size.
    '
    ' After this call, nuBASIC can use the Ray* API to load worlds, move the
    ' player, update the simulation and render the pseudo-3D scene.
    RayInit(viewW, viewH)

    ' Load the project/world description.
    '
    ' RayLoadProject() is the preferred path for the demo project format.
    ' RayLoadWorld() is kept as a fallback for direct world loading.
    If RayLoadProject(world) = 0 Then
        If RayLoadWorld(world) = 0 Then
            Print "Cannot load WinRayCast demo world from: "; scriptBase
            Main = 1
            Exit Function
        End If
    End If

    ' Store the initial player position as the first checkpoint.
    checkpointX = RayPlayerX()
    checkpointY = RayPlayerY()
    checkpointFacing = RayPlayerFacing()
    checkpointSaved = 1

    ' ------------------------------------------------------------------
    ' nuBASIC GRAPHICS WINDOW SETUP
    ' ------------------------------------------------------------------

    Screen 1

    ' The window contains the 3D viewport plus a 320-pixel side HUD.
    MoveWindow 40, 40, viewW + 320, viewH + 40

    ' Disable automatic redraw.
    ' The game loop will explicitly draw and refresh one complete frame at a time.
    ScreenLock

    ' ------------------------------------------------------------------
    ' MAIN GAME LOOP
    ' ------------------------------------------------------------------

    shouldQuit = 0

    While shouldQuit = 0
        ' Global quit shortcuts.
        If RayKeyDown(vkEscape) = 1 Or RayKeyDown(vkQ) = 1 Then shouldQuit = 1

        ' Used to charge movement energy only when the player actually moves.
        moving = 0

        ' Shot cooldown prevents firing every frame.
        If shotCooldown > 0.0 Then shotCooldown = shotCooldown - delta

        If lifeState = 0 And gameDone = 0 Then
            ' ----------------------------------------------------------
            ' PLAYER INPUT
            '
            ' Movement and rotation are delegated to WinRayCast.
            ' nuBASIC only checks the keyboard and calls the corresponding
            ' Ray* primitive.
            ' ----------------------------------------------------------

            If RayKeyDown(vkLeft) = 1 Or RayKeyDown(vkA) = 1 Then
                RayTurn(turnLeftStep)
            End If

            If RayKeyDown(vkRight) = 1 Or RayKeyDown(vkD) = 1 Then
                RayTurn(turnStep)
            End If

            If RayKeyDown(vkUp) = 1 Or RayKeyDown(vkW) = 1 Then
                RayMove(moveStep)
                moving = 1
            End If

            If RayKeyDown(vkDown) = 1 Or RayKeyDown(vkS) = 1 Then
                RayMove(backStep)
                moving = 1
            End If

            ' Fire only if the cooldown has expired.
            If shotCooldown <= 0.0 Then
                If RayKeyDown(vkSpace) = 1 Or RayKeyDown(vkCtrl) = 1 Then
                    ' Damage enemies or shootable objects inside the frontal cone.
                    '
                    ' WinRayCast performs the spatial/raycast check internally.
                    ' BASIC only provides damage, range and field-of-view values.
                    RayDamageEnemy(fireDamage, fireRangeCells, fireFovDegrees)

                    shotCooldown = fireCooldownSeconds
                    energy = energy - energyShotCost
                End If
            End If

            ' No passive energy drain: energy is consumed by actions and damage only.
            If moving = 1 Then energy = energy - energyMoveCost

            ' Enter death state when energy reaches zero.
            If energy <= 0.0 Then
                energy = 0.0
                lifeState = 1
                deathTimer = 0.0
            End If

            ' ----------------------------------------------------------
            ' AUTOSAVE
            '
            ' The checkpoint stores only the player position and facing.
            ' This keeps the demo simple while still showing how BASIC-side
            ' game logic can cooperate with the engine state.
            ' ----------------------------------------------------------

            autosaveTimer = autosaveTimer + delta

            If autosaveTimer >= autosaveSeconds Then
                checkpointX = RayPlayerX()
                checkpointY = RayPlayerY()
                checkpointFacing = RayPlayerFacing()
                checkpointSaved = 1
                autosaveTimer = 0.0
            End If
        Else
            ' ----------------------------------------------------------
            ' DEATH / RESPAWN STATE
            '
            ' While dead, input is ignored.
            ' After a delay, the player is restored to the last checkpoint.
            ' ----------------------------------------------------------

            deathTimer = deathTimer + delta

            If deathTimer >= respawnSeconds Then
                RaySetPlayer(checkpointX, checkpointY, checkpointFacing)

                energy = 100.0
                lifeState = 0
                deathTimer = 0.0
                autosaveTimer = 0.0
            End If
        End If

        ' --------------------------------------------------------------
        ' ENGINE UPDATE
        '
        ' Advances the internal WinRayCast simulation:
        ' - animations
        ' - doors
        ' - pickups
        ' - enemies
        ' - elevators
        ' - music
        ' - damage events
        ' --------------------------------------------------------------

        RayUpdate(delta)

        ' Enemy attacks are accumulated inside the engine.
        ' BASIC consumes the accumulated damage once per frame and applies it
        ' to the player energy.
        If lifeState = 0 And gameDone = 0 Then
            incomingDamage = RayConsumePlayerDamage()
            If incomingDamage > 0.0 Then energy = energy - incomingDamage
        End If

        ' --------------------------------------------------------------
        ' RENDERING PIPELINE
        '
        ' RayRender() draws the pseudo-3D scene into WinRayCast's internal
        ' software framebuffer.
        '
        ' RayPresent() copies that framebuffer into the nuBASIC graphics window.
        '
        ' This is the main integration point between the raycasting engine
        ' and the BASIC graphics environment.
        ' --------------------------------------------------------------

        RayRender(viewW, viewH)
        RayPresent(0, 0, viewW, viewH)

        ' Check mission completion.
        enemies = RayEnemyCount()
        killed = RayKilledEnemyCount()

        If enemies > 0 And killed >= enemies Then gameDone = 1

        ' Draw the BASIC-side HUD over/next to the rendered scene.
        energyInt = energy
        DrawHud viewW, viewH, hudX, energyInt, autosaveTimer, autosaveSeconds, checkpointSaved, gameDone

        ' Draw death overlay if needed.
        If lifeState <> 0 Then
            redStrength = (deathTimer * 100) / deathOverlaySeconds
            DrawDeathOverlay viewW, viewH, redStrength, 0
        End If

        ' Draw mission-complete message.
        If gameDone = 1 Then
            TextOut 410, 350, "MISSION COMPLETE", Rgb(160, 255, 160)
        End If

        ' Show the complete frame.
        Refresh

        ' One-frame mode, useful for smoke tests or screenshots.
        If once = 1 Then shouldQuit = 1

        ' Small delay to keep the loop close to 60 FPS.
        MDelay(16)
    Wend

    ' Restore standard nuBASIC graphics behaviour.
    ScreenUnlock

    Main = 0
End Function

