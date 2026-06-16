' Eclipse Protocol.
'
' This file is intentionally written as a tutorial.
' The GAME SETTINGS section contains the parameters that are meant to be changed,
' while the main loop shows how a nuBASIC program can drive the WinRayCast engine.
'
' WinRayCast provides the pseudo-3D raycasting engine.
' nuBASIC provides the scripting layer, graphics window, HUD drawing and game logic.
Syntax Modern

Using raycast
Using graphics
Using math
Using String
Using system

Struct GameSettings
    ViewW As Integer
    ViewH As Integer
    HudX As Integer
    FrameBudgetMs As Integer
    MoveStep As Integer
    TurnStep As Double
    EnergyIdleCost As Double
    EnergyMoveCost As Double
    EnergyShotCost As Double
    AutosaveSeconds As Double
    RespawnSeconds As Double
    DeathOverlaySeconds As Double
    HitFlashSeconds As Double
    ViewCenterLift As Double
    ViewCenterEase As Double
End Struct

Struct KeyBindings
    Escape As Integer
    Enter As Integer
    LeftKey As Integer
    UpKey As Integer
    RightKey As Integer
    DownKey As Integer
    A As Integer
    D As Integer
    Q As Integer
    R As Integer
    S As Integer
    W As Integer
    SpaceKey As Integer
    Ctrl As Integer
    Weapon1 As Integer
    Weapon2 As Integer
    Weapon3 As Integer
    EasyMode As Integer
    HardMode As Integer
    LookUp As Integer
    LookDown As Integer
End Struct

Class EventLog
    Public Line1 As String
    Public Line2 As String
    Public Line3 As String
    Public Line4 As String

    Public Sub Push(message As String)
        Me.Line4 = Me.Line3
        Me.Line3 = Me.Line2
        Me.Line2 = Me.Line1
        Me.Line1 = message
    End Sub
End Class

' Keeps all tweakable game settings in one visible place.
Sub ConfigureGame(ByRef settings As GameSettings)
    settings.ViewW = 1024              ' 3D viewport width.
    settings.ViewH = 1024              ' 3D viewport height.
    settings.HudX = settings.ViewW + 16 ' Initial X position of the side HUD.
    settings.FrameBudgetMs = 16        ' Target frame duration for the 60 FPS cap.
    settings.MoveStep = 28             ' Forward movement step per frame.
    settings.TurnStep = 3.0            ' Rotation step in degrees per frame.
    settings.EnergyIdleCost = 0.001    ' Minimal idle energy drain per frame.
    settings.EnergyMoveCost = 0.001    ' Extra energy cost per frame while moving.
    settings.EnergyShotCost = 0.05     ' Energy cost per pistol shot.
    settings.AutosaveSeconds = 60.0    ' Time between automatic checkpoints.
    settings.RespawnSeconds = 3.25     ' Delay before restoring from autosave.
    settings.DeathOverlaySeconds = 2.0 ' Red overlay ramp-up duration.
    settings.HitFlashSeconds = 0.18    ' Short red viewport flash after damage.
    settings.ViewCenterLift = 0.055    ' Camera lift on supply/toolbox items.
    settings.ViewCenterEase = 0.2      ' Smooth camera interpolation amount.
End Sub

' Windows virtual-key codes used by RayKeyDown().
Sub ConfigureKeys(ByRef keys As KeyBindings)
    keys.Escape = 27
    keys.Enter = 13
    keys.LeftKey = 37
    keys.UpKey = 38
    keys.RightKey = 39
    keys.DownKey = 40
    keys.A = 65
    keys.D = 68
    keys.Q = 81
    keys.R = 82
    keys.S = 83
    keys.W = 87
    keys.SpaceKey = 32
    keys.Ctrl = 17
    keys.Weapon1 = 49
    keys.Weapon2 = 50
    keys.Weapon3 = 51
    keys.EasyMode = 112
    keys.HardMode = 113
    keys.LookUp = 33
    keys.LookDown = 34
End Sub

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

' Draws the stable end-of-mission screen.
'
' This screen intentionally does not call RayRender() or RayPresent().
' Once the mission is complete the game switches to a small menu loop that
' redraws only this neutral image, avoiding flicker from the live 3D scene.
Sub DrawMissionCompleteScreen(totalW As Integer, viewH As Integer)
    Dim cx As Integer
    Dim cy As Integer
    Dim panelW As Integer
    Dim panelH As Integer
    Dim x1 As Integer
    Dim y1 As Integer
    Dim edgeSize As Integer

    cx = totalW / 2
    cy = viewH / 2
    panelW = 620
    panelH = 190
    x1 = cx - panelW / 2
    y1 = cy - panelH / 2
    edgeSize = viewH / 18

    ' Neutral background with a green-blue completion tint.
    FillRect 0, 0, totalW, viewH, Rgb(8, 14, 18)
    FillRect 0, 0, totalW, edgeSize, Rgb(0, 70, 86)
    FillRect 0, viewH - edgeSize, totalW, viewH, Rgb(0, 70, 86)
    FillRect 0, 0, edgeSize, viewH, Rgb(0, 84, 70)
    FillRect totalW - edgeSize, 0, totalW, viewH, Rgb(0, 84, 70)

    ' Centered message panel.
    FillRect x1, y1, x1 + panelW, y1 + panelH, Rgb(12, 24, 24)
    Rect x1, y1, x1 + panelW, y1 + panelH, Rgb(105, 225, 185)
    Rect x1 + 6, y1 + 6, x1 + panelW - 6, y1 + panelH - 6, Rgb(45, 125, 150)

    TextOut cx - 142, cy - 58, "ECLIPSE PROTOCOL COMPLETE", Rgb(170, 255, 210)
    TextOut cx - 124, cy - 18, "Congratulations, commander.", Rgb(230, 245, 240)
    TextOut cx - 156, cy + 22, "Press R to replay the mission", Rgb(190, 225, 220)
    TextOut cx - 116, cy + 48, "Press Q or Esc to exit", Rgb(160, 195, 210)
End Sub

' Draws a short red border when an enemy successfully hits the player.
Sub DrawHitOverlay(viewW As Integer, viewH As Integer)
    Dim edgeSize As Integer

    edgeSize = 26
    FillRect 0, 0, viewW, edgeSize, Rgb(190, 0, 0)
    FillRect 0, viewH - edgeSize, viewW, viewH, Rgb(190, 0, 0)
    FillRect 0, 0, edgeSize, viewH, Rgb(190, 0, 0)
    FillRect viewW - edgeSize, 0, viewW, viewH, Rgb(190, 0, 0)
End Sub

' Draws a compact weapon selector.
'
' The actual first-person weapon is rendered by WinRayCast in the 3D view.
' A bitmap preview can be added here later by loading the weapon idle frame.
Sub DrawWeaponPreview(hudX As Integer, y As Integer, selectedWeapon As Integer, weaponName As String)
    Dim slotW As Integer
    Dim x1 As Integer
    Dim activeColor As Integer
    Dim pistolAvailable As Integer
    Dim shotgunAvailable As Integer
    Dim smgAvailable As Integer

    slotW = 68
    pistolAvailable = RayHasWeapon("weapons/pistol/pistol.weapon.json")
    shotgunAvailable = RayHasWeapon("weapons/super_shotgun/super_shotgun.weapon.json")
    smgAvailable = RayHasWeapon("weapons/submachine_gun/submachine_gun.weapon.json")

    TextOut hudX, y, "WEAPON", Rgb(230, 230, 230)
    TextOut hudX, y + 24, weaponName, Rgb(190, 210, 235)

    ' Draw the three keyboard slots: 1 pistol, 2 shotgun, 3 SMG.
    x1 = hudX
    While x1 < hudX + slotW * 3
        FillRect x1, y + 52, x1 + slotW - 6, y + 86, Rgb(34, 38, 46)
        Rect x1, y + 52, x1 + slotW - 6, y + 86, Rgb(90, 100, 116)
        x1 = x1 + slotW
    Wend

    If pistolAvailable = 1 Then
        TextOut hudX + 24, y + 60, "1", Rgb(220, 220, 220)
    Else
        TextOut hudX + 24, y + 60, "-", Rgb(95, 100, 110)
    End If

    If shotgunAvailable = 1 Then
        TextOut hudX + slotW + 24, y + 60, "2", Rgb(220, 220, 220)
    Else
        TextOut hudX + slotW + 24, y + 60, "-", Rgb(95, 100, 110)
    End If

    If smgAvailable = 1 Then
        TextOut hudX + slotW * 2 + 24, y + 60, "3", Rgb(220, 220, 220)
    Else
        TextOut hudX + slotW * 2 + 24, y + 60, "-", Rgb(95, 100, 110)
    End If

    activeColor = Rgb(115, 190, 255)
    If selectedWeapon = 1 Then Rect hudX - 2, y + 50, hudX + slotW - 4, y + 88, activeColor
    If selectedWeapon = 2 Then Rect hudX + slotW - 2, y + 50, hudX + slotW * 2 - 4, y + 88, activeColor
    If selectedWeapon = 3 Then Rect hudX + slotW * 2 - 2, y + 50, hudX + slotW * 3 - 4, y + 88, activeColor
End Sub

' Draws a local minimap around the player.
'
' The engine exposes only map queries; BASIC decides how much of the map to
' draw and how to style it. This keeps the minimap easy to customize.
Sub DrawMiniMap(hudX As Integer, y As Integer, mapUnlocks As Integer)
    Dim cellDx As Integer
    Dim cellDy As Integer
    Dim rows As Integer
    Dim cols As Integer
    Dim playerCol As Integer
    Dim playerRow As Integer
    Dim radius As Integer
    Dim tile As Integer
    Dim dx As Integer
    Dim dy As Integer
    Dim mapRow As Integer
    Dim mapCol As Integer
    Dim x As Integer
    Dim yy As Integer
    Dim color As Integer
    Dim centerX As Integer
    Dim centerY As Integer
    Dim gridSize As Integer
    Dim gridX As Integer
    Dim gridY As Integer
    Dim panelX As Integer
    Dim panelY As Integer
    Dim cellKind As Integer
    Dim keyKind As Integer
    Dim angleRad As Double
    Dim facingX As Integer
    Dim facingY As Integer

    radius = 5
    tile = 14
    gridSize = (radius * 2 + 1) * tile
    panelX = hudX + 25
    panelY = y + 24
    gridX = panelX + 8
    gridY = panelY + 8
    centerX = gridX + radius * tile + tile / 2
    centerY = gridY + radius * tile + tile / 2

    TextOut hudX, y, "MAP", Rgb(230, 230, 230)
    FillRect panelX, panelY, panelX + gridSize + 16, panelY + gridSize + 16, Rgb(12, 14, 18)
    Rect panelX, panelY, panelX + gridSize + 16, panelY + gridSize + 16, Rgb(78, 88, 105)

    If mapUnlocks < 1 Then
        TextOut panelX + 38, panelY + 68, "MAP OFFLINE", Rgb(130, 140, 155)
        TextOut panelX + 28, panelY + 92, "Find a computer", Rgb(90, 100, 115)
        Exit Sub
    End If

    cellDx = RayCellDx()
    cellDy = RayCellDy()
    rows = RayMapRows()
    cols = RayMapCols()
    If cellDx <= 0 Or cellDy <= 0 Then Exit Sub

    playerCol = RayPlayerX() / cellDx
    playerRow = RayPlayerY() / cellDy

    For dy = 0 - radius To radius
        For dx = 0 - radius To radius
            mapRow = playerRow + dy
            mapCol = playerCol + dx
            x = gridX + (dx + radius) * tile
            yy = gridY + (dy + radius) * tile

            If mapRow < 0 Or mapCol < 0 Or mapRow >= rows Or mapCol >= cols Then
                color = Rgb(24, 24, 28)
            Else
                cellKind = RayCellKind(mapRow, mapCol)
                If cellKind = 2 Then
                    color = Rgb(180, 135, 70)
                ElseIf cellKind = 1 Then
                    color = Rgb(80, 88, 100)
                Else
                    color = Rgb(30, 52, 46)
                End If
            End If

            FillRect x, yy, x + tile, yy + tile, color

            ' After the second computer item, uncollected keys appear on the map.
            If mapUnlocks >= 2 Then
                keyKind = RayKeyAtCell(mapRow, mapCol)
                If keyKind = 1 Then FillRect x + 4, yy + 4, x + tile - 4, yy + tile - 4, Rgb(80, 230, 120)
                If keyKind = 2 Then FillRect x + 4, yy + 4, x + tile - 4, yy + tile - 4, Rgb(230, 80, 80)
                If keyKind = 3 Then FillRect x + 4, yy + 4, x + tile - 4, yy + tile - 4, Rgb(80, 140, 255)
            End If
        Next dx
    Next dy

    ' Player marker and facing vector.
    FillRect centerX - 3, centerY - 3, centerX + 4, centerY + 4, Rgb(245, 235, 150)
    angleRad = RayPlayerFacing() * 3.14159265 / 180.0
    facingX = centerX + Cos(angleRad) * 20.0
    facingY = centerY + Sin(angleRad) * 20.0
    Line centerX, centerY, facingX, facingY, Rgb(245, 235, 150)
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
Sub DrawHud(viewW As Integer, _
            viewH As Integer, _
            hudX As Integer, _
            energy As Integer, _
            autosaveTimer As Double, _
            autosaveSeconds As Double, _
            saved As Integer, _
            gameDone As Integer, _
            selectedWeapon As Integer, _
            weaponName As String, _
            weaponAmmo As Integer, _
            weaponReserve As Integer, _
            mapUnlocks As Integer, _
            difficultyName As String, _
            eventLog As EventLog)
    Dim enemies As Integer
    Dim killed As Integer
    Dim killPct As Integer
    Dim itemPct As Integer
    Dim items As Integer
    Dim collected As Integer
    Dim autoRemain As Integer
    Dim elevatorOptions As Integer
    Dim elevatorSelected As Integer
    Dim elevatorTarget As String

    ' Query the current game state from the raycast engine.
    enemies = RayEnemyCount()
    killed = RayKilledEnemyCount()
    items = RayItemCount()
    collected = RayCollectedItemCount()

    killPct = Percent(killed, enemies)
    itemPct = Percent(collected, items)

    ' HUD background.
    FillRect viewW, 0, viewW + 320, viewH, Rgb(16, 18, 22)

    TextOut hudX, 24, "Eclipse Protocol", Rgb(235, 235, 235)

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

    TextOut hudX, 424, "Difficulty: " + difficultyName, Rgb(190, 195, 205)

    elevatorOptions = RayTransitionOptionCount()
    If elevatorOptions > 0 Then
        elevatorSelected = RayTransitionSelected()
        elevatorTarget = RayTransitionOptionLayer$(elevatorSelected)
        If RayTransitionActive() = 1 Then
            TextOut hudX, 448, "Elevator moving to " + elevatorTarget, Rgb(245, 220, 130)
        Else
            TextOut hudX, 448, "Elevator target " + elevatorTarget + " < > Enter", Rgb(245, 220, 130)
        End If
    End If

    ' Controls.
    TextOut hudX, 466, "W/S or arrows: move", Rgb(170, 180, 190)
    TextOut hudX, 490, "A/D or arrows: turn", Rgb(170, 180, 190)
    TextOut hudX, 514, "PgUp/PgDn: look", Rgb(170, 180, 190)
    TextOut hudX, 538, "Space/Ctrl: fire", Rgb(170, 180, 190)
    TextOut hudX, 562, "R: reload", Rgb(170, 180, 190)
    TextOut hudX, 586, "1/2/3: weapon", Rgb(170, 180, 190)
    TextOut hudX, 610, "F1/F2: difficulty", Rgb(170, 180, 190)

    DrawWeaponPreview hudX, 626, selectedWeapon, weaponName
    TextOut hudX, 716, "Ammo " + Str$(weaponAmmo) + " / " + Str$(weaponReserve), Rgb(190, 210, 235)
    DrawMiniMap hudX, 744, mapUnlocks

    ' Event log. Keeping it in the side panel avoids covering the 3D view.
    TextOut hudX, 944, "EVENT LOG", Rgb(230, 230, 230)
    TextOut hudX, 966, eventLog.Line1, Rgb(190, 210, 235)
    TextOut hudX, 984, eventLog.Line2, Rgb(170, 190, 215)
    TextOut hudX, 1002, eventLog.Line3, Rgb(150, 170, 195)
    TextOut hudX, 1020, eventLog.Line4, Rgb(130, 150, 175)
End Sub

Function Main(argc As Integer, argv() As String) As Integer
    Dim viewW As Integer
    Dim viewH As Integer
    Dim hudX As Integer
    Dim moveStep As Integer
    Dim backStep As Integer
    Dim turnStep As Double
    Dim turnLeftStep As Double
    Dim settings As GameSettings
    Dim keys As KeyBindings
    Dim eventLog As New EventLog()
    Dim scriptBase As String
    Dim world As String
    Dim weaponPath As String
    Dim weaponName As String
    Dim shouldQuit As Integer
    Dim once As Integer
    Dim energy As Double
    Dim energyInt As Integer
    Dim delta As Double
    Dim frameBudgetMs As Integer
    Dim frameStartMs As Integer
    Dim frameElapsedMs As Integer
    Dim frameDelayMs As Integer
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
    Dim shotResult As Integer
    Dim incomingDamage As Double
    Dim incomingHealing As Double
    Dim autosaveSeconds As Double
    Dim respawnSeconds As Double
    Dim deathOverlaySeconds As Double
    Dim energyIdleCost As Double
    Dim energyMoveCost As Double
    Dim energyShotCost As Double
    Dim fireDamage As Double
    Dim fireRangeCells As Double
    Dim fireFovDegrees As Double
    Dim fireCooldownSeconds As Double
    Dim lastCollectedItems As Integer
    Dim collectedItems As Integer
    Dim lastDestroyedObjects As Integer
    Dim destroyedObjects As Integer
    Dim lastLayer As String
    Dim currentLayer As String
    Dim selectedWeapon As Integer
    Dim mapUnlocks As Integer
    Dim lastWeaponKey1 As Integer
    Dim lastWeaponKey2 As Integer
    Dim lastWeaponKey3 As Integer
    Dim lastReloadKey As Integer
    Dim lastElevatorLeftKey As Integer
    Dim lastElevatorRightKey As Integer
    Dim lastElevatorEnterKey As Integer
    Dim lastRestartKey As Integer
    Dim lastDifficultyKey1 As Integer
    Dim lastDifficultyKey2 As Integer
    Dim weaponLoaded As Integer
    Dim difficultyMode As Integer
    Dim difficultyName As String
    Dim enemyDamageScale As Double
    Dim playerSlope As Integer
    Dim slopeStep As Integer
    Dim playerViewCenter As Double
    Dim targetViewCenter As Double
    Dim viewCenterLift As Double
    Dim viewCenterEase As Double
    Dim hitFlashTimer As Double
    Dim hitFlashSeconds As Double
    Dim weaponAmmo As Integer
    Dim weaponReserve As Integer
    Dim elevatorOptions As Integer
    Dim elevatorSelected As Integer
    Dim screenW As Integer

    ' ------------------------------------------------------------------
    ' GAME SETTINGS
    '
    ' Change these values to alter the behaviour of the demo without
    ' touching the rest of the program.
    ' ------------------------------------------------------------------

    ConfigureGame(settings)
    ConfigureKeys(keys)

    delta = 0.016              ' Simulated frame delta, roughly 60 FPS.
    ' Used by animations, doors, enemies and timers.
    frameBudgetMs = settings.FrameBudgetMs

    viewW = settings.ViewW
    viewH = settings.ViewH
    hudX = settings.HudX
    screenW = viewW + 320

    moveStep = settings.MoveStep
    backStep = 0 - moveStep    ' Negative movement step for walking backwards.

    turnStep = settings.TurnStep
    turnLeftStep = 0.0 - turnStep

    energy = 100.0             ' Initial player energy.

    energyIdleCost = settings.EnergyIdleCost
    energyMoveCost = settings.EnergyMoveCost
    energyShotCost = settings.EnergyShotCost

    difficultyMode = 0         ' 0 = easy, 1 = hard.
    difficultyName = "Easy"
    enemyDamageScale = 0.6    ' Easy mode reduces enemy weapon damage.

    playerSlope = 0            ' Vertical look offset controlled with PageUp/PageDown.
    slopeStep = 8              ' Slope pixels added or removed for each pressed frame.
    playerViewCenter = 0.5    ' Normal camera height ratio used by the raycaster.
    targetViewCenter = 0.5
    viewCenterLift = settings.ViewCenterLift
    viewCenterEase = settings.ViewCenterEase

    autosaveSeconds = settings.AutosaveSeconds
    respawnSeconds = settings.RespawnSeconds
    deathOverlaySeconds = settings.DeathOverlaySeconds
    hitFlashSeconds = settings.HitFlashSeconds

    fireDamage = 18.0          ' Base damage dealt by the selected weapon.
    fireRangeCells = 8.5       ' Maximum shooting range, expressed in map cells.
    fireFovDegrees = 22.0      ' Front cone angle used by the shot.
    fireCooldownSeconds = 0.32 ' Minimum time between two shots.
    selectedWeapon = 1         ' The demo starts with the pistol selected.
    weaponName = "Pistol"      ' Current weapon label shown in the HUD.
    weaponPath = "weapons/pistol/pistol.weapon.json"

    ' Start counting from zero: the first automatic checkpoint is one minute later.
    autosaveTimer = 0.0

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

    ' The demo project has elevators with multiple possible destinations.
    ' Manual selection keeps the player in control instead of always taking
    ' the first transition declared in the JSON project file.
    RaySetTransitionManual(1)

    ' Store the initial player position as the first checkpoint.
    checkpointX = RayPlayerX()
    checkpointY = RayPlayerY()
    checkpointFacing = RayPlayerFacing()
    checkpointSaved = 1
    lastCollectedItems = RayCollectedItemCount()
    lastDestroyedObjects = RayDestroyedObjectCount()
    lastLayer = RayCurrentLayer$()
    eventLog.Line1 = "World loaded: " + lastLayer
    eventLog.Line2 = "Autosave ready"
    eventLog.Line3 = "Keys 1/2/3 switch weapons"
    eventLog.Line4 = "Find computers to unlock the map"

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
        frameStartMs = Millis()

        ' Global quit shortcuts.
        If RayKeyDown(keys.Escape) = 1 Or RayKeyDown(keys.Q) = 1 Then shouldQuit = 1

        ' Mission-complete menu.
        '
        ' The completed mission uses its own stable rendering loop. It stops
        ' the live raycast presentation and redraws only a neutral end screen
        ' while waiting for replay/exit input.
        If gameDone = 1 Then
            While gameDone = 1 And shouldQuit = 0
                frameStartMs = Millis()

                If RayKeyDown(keys.Escape) = 1 Or RayKeyDown(keys.Q) = 1 Then shouldQuit = 1

                If RayKeyDown(keys.R) = 1 Then
                    If lastRestartKey = 0 Then
                        If RayLoadProject(world) = 1 Then
                            RaySetTransitionManual(1)
                            energy = 100.0
                            lifeState = 0
                            deathTimer = 0.0
                            autosaveTimer = 0.0
                            gameDone = 0
                            selectedWeapon = 1
                            weaponName = "Pistol"
                            weaponPath = "weapons/pistol/pistol.weapon.json"
                            fireDamage = 18.0
                            fireRangeCells = 8.5
                            fireFovDegrees = 22.0
                            fireCooldownSeconds = 0.32
                            energyShotCost = settings.EnergyShotCost
                            checkpointX = RayPlayerX()
                            checkpointY = RayPlayerY()
                            checkpointFacing = RayPlayerFacing()
                            checkpointSaved = 1
                            lastCollectedItems = RayCollectedItemCount()
                            lastDestroyedObjects = RayDestroyedObjectCount()
                            lastLayer = RayCurrentLayer$()
                            eventLog.Line1 = "Mission restarted"
                            eventLog.Line2 = "World loaded: " + lastLayer
                            eventLog.Line3 = "Autosave ready"
                            eventLog.Line4 = "Find computers to unlock the map"
                        Else
                            eventLog.Push("Cannot restart mission")
                        End If
                    End If
                    lastRestartKey = 1
                Else
                    lastRestartKey = 0
                End If

                DrawMissionCompleteScreen screenW, viewH
                Refresh

                frameElapsedMs = Millis() - frameStartMs
                frameDelayMs = frameBudgetMs - frameElapsedMs
                If frameDelayMs > 0 Then MDelay(frameDelayMs)
            Wend
        End If

        If shouldQuit = 1 Then Exit While

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

            elevatorOptions = RayTransitionOptionCount()

            If elevatorOptions > 0 And RayTransitionActive() = 0 Then
                If RayKeyDown(keys.LeftKey) = 1 Or RayKeyDown(keys.A) = 1 Then
                    If lastElevatorLeftKey = 0 Then
                        elevatorSelected = RaySelectTransition(RayTransitionSelected() - 1)
                        eventLog.Push("Elevator target: " + RayTransitionOptionLayer$(elevatorSelected))
                    End If
                    lastElevatorLeftKey = 1
                Else
                    lastElevatorLeftKey = 0
                End If

                If RayKeyDown(keys.RightKey) = 1 Or RayKeyDown(keys.D) = 1 Then
                    If lastElevatorRightKey = 0 Then
                        elevatorSelected = RaySelectTransition(RayTransitionSelected() + 1)
                        eventLog.Push("Elevator target: " + RayTransitionOptionLayer$(elevatorSelected))
                    End If
                    lastElevatorRightKey = 1
                Else
                    lastElevatorRightKey = 0
                End If

                If RayKeyDown(keys.Enter) = 1 Then
                    If lastElevatorEnterKey = 0 Then
                        If RayConfirmTransition() = 1 Then
                            eventLog.Push("Elevator moving")
                        End If
                    End If
                    lastElevatorEnterKey = 1
                Else
                    lastElevatorEnterKey = 0
                End If
            Else
                lastElevatorLeftKey = 0
                lastElevatorRightKey = 0
                lastElevatorEnterKey = 0

                If RayTransitionActive() = 0 Then
                    If RayKeyDown(keys.LeftKey) = 1 Or RayKeyDown(keys.A) = 1 Then
                        RayTurn(turnLeftStep)
                    End If

                    If RayKeyDown(keys.RightKey) = 1 Or RayKeyDown(keys.D) = 1 Then
                        RayTurn(turnStep)
                    End If
                End If
            End If

            If RayTransitionActive() = 0 Then
                If RayKeyDown(keys.UpKey) = 1 Or RayKeyDown(keys.W) = 1 Then
                    RayMove(moveStep)
                    moving = 1
                End If

                If RayKeyDown(keys.DownKey) = 1 Or RayKeyDown(keys.S) = 1 Then
                    RayMove(backStep)
                    moving = 1
                End If
            End If

            ' Difficulty is BASIC-side game balance.
            ' The engine reports raw enemy damage; the script scales it below
            ' before applying it to the player energy.
            If RayKeyDown(keys.EasyMode) = 1 Then
                If lastDifficultyKey1 = 0 Then
                    difficultyMode = 0
                    difficultyName = "Easy"
                    enemyDamageScale = 0.6
                    eventLog.Push("Difficulty: Easy")
                End If
                lastDifficultyKey1 = 1
            Else
                lastDifficultyKey1 = 0
            End If

            If RayKeyDown(keys.HardMode) = 1 Then
                If lastDifficultyKey2 = 0 Then
                    difficultyMode = 1
                    difficultyName = "Hard"
                    enemyDamageScale = 1.35
                    eventLog.Push("Difficulty: Hard")
                End If
                lastDifficultyKey2 = 1
            Else
                lastDifficultyKey2 = 0
            End If

            ' The raycast engine exposes the vertical view slope directly.
            ' Positive values look lower, negative values look higher.
            If RayKeyDown(keys.LookUp) = 1 Then playerSlope = playerSlope - slopeStep
            If RayKeyDown(keys.LookDown) = 1 Then playerSlope = playerSlope + slopeStep
            RaySetPlayerSlope(playerSlope)
            playerSlope = RayPlayerSlope()

            If RayKeyDown(keys.R) = 1 Then
                If lastReloadKey = 0 Then
                    If RayReloadWeapon() = 1 Then
                        eventLog.Push("Reloading " + weaponName)
                    Else
                        eventLog.Push("No reload needed")
                    End If
                End If
                lastReloadKey = 1
            Else
                lastReloadKey = 0
            End If

            ' Weapon switching is BASIC-side game logic.
            '
            ' RayLoadWeapon() swaps the first-person weapon rendered by the
            ' engine. The BASIC script keeps matching damage, range and timing
            ' values so the tutorial can show the full gameplay loop.
            If RayKeyDown(keys.Weapon1) = 1 Then
                If lastWeaponKey1 = 0 And selectedWeapon <> 1 Then
                    weaponPath = "weapons/pistol/pistol.weapon.json"
                    weaponLoaded = RayLoadWeapon(weaponPath)
                    If weaponLoaded = 1 Then
                        selectedWeapon = 1
                        weaponName = "Pistol"
                        fireDamage = 18.0
                        fireRangeCells = 8.5
                        fireFovDegrees = 22.0
                        fireCooldownSeconds = 0.32
                        energyShotCost = 0.05
                        eventLog.Push("Weapon selected: " + weaponName)
                    Else
                        eventLog.Push("Weapon locked: Pistol")
                    End If
                End If
                lastWeaponKey1 = 1
            Else
                lastWeaponKey1 = 0
            End If

            If RayKeyDown(keys.Weapon2) = 1 Then
                If lastWeaponKey2 = 0 And selectedWeapon <> 2 Then
                    weaponPath = "weapons/super_shotgun/super_shotgun.weapon.json"
                    weaponLoaded = RayLoadWeapon(weaponPath)
                    If weaponLoaded = 1 Then
                        selectedWeapon = 2
                        weaponName = "Super Shotgun"
                        fireDamage = 45.0
                        fireRangeCells = 7.5
                        fireFovDegrees = 34.0
                        fireCooldownSeconds = 0.72
                        energyShotCost = 0.16
                        eventLog.Push("Weapon selected: " + weaponName)
                    Else
                        eventLog.Push("Weapon locked: Super Shotgun")
                    End If
                End If
                lastWeaponKey2 = 1
            Else
                lastWeaponKey2 = 0
            End If

            If RayKeyDown(keys.Weapon3) = 1 Then
                If lastWeaponKey3 = 0 And selectedWeapon <> 3 Then
                    weaponPath = "weapons/submachine_gun/submachine_gun.weapon.json"
                    weaponLoaded = RayLoadWeapon(weaponPath)
                    If weaponLoaded = 1 Then
                        selectedWeapon = 3
                        weaponName = "Submachine Gun"
                        fireDamage = 10.0
                        fireRangeCells = 7.0
                        fireFovDegrees = 24.0
                        fireCooldownSeconds = 0.125
                        energyShotCost = 0.02
                        eventLog.Push("Weapon selected: " + weaponName)
                    Else
                        eventLog.Push("Weapon locked: Submachine Gun")
                    End If
                End If
                lastWeaponKey3 = 1
            Else
                lastWeaponKey3 = 0
            End If

            ' Fire only if the cooldown has expired.
            If shotCooldown <= 0.0 Then
                If RayKeyDown(keys.SpaceKey) = 1 Or RayKeyDown(keys.Ctrl) = 1 Then
                    ' Damage enemies or shootable objects inside the frontal cone.
                    '
                    ' WinRayCast performs the spatial/raycast check internally.
                    ' BASIC only provides damage, range and field-of-view values.
                    If RayWeaponAmmo() > 0 Or RayWeaponMaxAmmo() = 0 Then
                        shotResult = RayDamageEnemy(fireDamage, fireRangeCells, fireFovDegrees)
                        shotCooldown = fireCooldownSeconds
                        energy = energy - energyShotCost
                        eventLog.Push(weaponName + " fired")
                    Else
                        If RayReloadWeapon() = 1 Then
                            eventLog.Push("Reloading " + weaponName)
                        Else
                            eventLog.Push("No ammo")
                        End If
                        shotCooldown = fireCooldownSeconds
                    End If
                End If
            End If

            ' Energy drains very slowly at rest, then faster while moving.
            energy = energy - energyIdleCost
            If moving = 1 Then energy = energy - energyMoveCost

            ' Enter death state when energy reaches zero.
            If energy <= 0.0 Then
                energy = 0.0
                lifeState = 1
                deathTimer = 0.0
                eventLog.Push("Energy depleted")
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
                eventLog.Push("Autosave checkpoint stored")
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
                eventLog.Push("Respawned from autosave")
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
        ' - first-person weapon bobbing while the player is moving
        ' --------------------------------------------------------------

        RaySetPlayerEnergy(energy, 100.0)
        RayUpdate(delta, moving)

        ' Enemy attacks are accumulated inside the engine.
        ' BASIC consumes the accumulated damage once per frame and applies it
        ' to the player energy.
        If lifeState = 0 And gameDone = 0 Then
            incomingDamage = RayConsumePlayerDamage()
            If incomingDamage > 0.0 Then
                incomingDamage = incomingDamage * enemyDamageScale
                energy = energy - incomingDamage
                hitFlashTimer = hitFlashSeconds
                eventLog.Push("Enemy hit: -" + Str$(incomingDamage))
            End If

            incomingHealing = RayConsumePlayerHealing()
            If incomingHealing > 0.0 Then
                energy = energy + incomingHealing
                If energy > 100.0 Then energy = 100.0
                eventLog.Push("Medikit restored +" + Str$(incomingHealing))
            End If
        End If

        collectedItems = RayCollectedItemCount()
        If collectedItems > lastCollectedItems Then
            eventLog.Push("Item collected")
            lastCollectedItems = collectedItems
        End If

        destroyedObjects = RayDestroyedObjectCount()
        If destroyedObjects > lastDestroyedObjects Then
            eventLog.Push("Object destroyed")
            lastDestroyedObjects = destroyedObjects
        End If

        currentLayer = RayCurrentLayer$()
        If currentLayer <> lastLayer Then
            eventLog.Push("Elevator arrived: " + currentLayer)
            lastLayer = currentLayer
        End If

        mapUnlocks = RayMapUnlockCount()
        If hitFlashTimer > 0.0 Then hitFlashTimer = hitFlashTimer - delta

        ' Raise the camera slightly while standing on low props.
        ' This uses the raycaster projection center rather than moving the
        ' player through the map, so collision and pickups remain unchanged.
        If RayTransitionActive() = 1 Then
            ' During elevator movement the engine shakes the camera directly.
            ' Keep the BASIC-side smoothing state in sync instead of overriding it.
            playerViewCenter = RayPlayerViewCenter()
        Else
            targetViewCenter = 0.5
            If RayPlayerStandingOn("supply_crate", 0.45) = 1 Then targetViewCenter = 0.5 + viewCenterLift
            If RayPlayerStandingOn("toolbox", 0.45) = 1 Then targetViewCenter = 0.5 + viewCenterLift
            playerViewCenter = playerViewCenter + (targetViewCenter - playerViewCenter) * viewCenterEase
            RaySetPlayerViewCenter(playerViewCenter)
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

        If enemies > 0 And killed >= enemies And gameDone = 0 Then
            gameDone = 1
            eventLog.Push("Congratulations, commander")
            eventLog.Push("R replay / Q exit")
        End If

        ' Draw the BASIC-side HUD over/next to the rendered scene.
        energyInt = energy
        weaponAmmo = RayWeaponAmmo()
        weaponReserve = RayWeaponReserveAmmo()
        DrawHud viewW, _
            viewH, _
            hudX, _
            energyInt, _
            autosaveTimer, _
            autosaveSeconds, _
            checkpointSaved, _
            gameDone, _
            selectedWeapon, _
            weaponName, _
            weaponAmmo, _
            weaponReserve, _
            mapUnlocks, _
            difficultyName, _
            eventLog

        If hitFlashTimer > 0.0 And lifeState = 0 Then
            DrawHitOverlay viewW, viewH
        End If

        ' Draw death overlay if needed.
        If lifeState <> 0 Then
            redStrength = (deathTimer * 100) / deathOverlaySeconds
            DrawDeathOverlay viewW, viewH, redStrength, 0
        End If

        ' Draw mission-complete message.
        ' The next loop iteration switches to the stable completion screen.
        If gameDone = 1 Then
            DrawMissionCompleteScreen screenW, viewH
        End If

        ' Show the complete frame.
        Refresh

        ' One-frame mode, useful for smoke tests or screenshots.
        If once = 1 Then shouldQuit = 1

        ' Sleep only the remaining frame budget.
        ' If rendering already took longer than 16 ms, skip the delay so the
        ' game does not add extra slowdown on top of an expensive frame.
        frameElapsedMs = Millis() - frameStartMs
        frameDelayMs = frameBudgetMs - frameElapsedMs
        If frameDelayMs > 0 Then MDelay(frameDelayMs)
    Wend

    ' Restore standard nuBASIC graphics behaviour.
    ScreenUnlock
    If RayShutdown() = 1 Then Main = 0

    Main = 1
End Function
