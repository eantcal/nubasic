Syntax Modern

' WinRayCast demo in nuBASIC.
' Questo file e' volutamente scritto come tutorial: la sezione GAME SETTINGS
' raccoglie i parametri da modificare, mentre il loop principale mostra come
' chiamare l'engine raycast da BASIC.

Using raycast
Using graphics
Using String
Using system

' Calcola una percentuale intera usata dalla HUD in stile Doom.
Function Percent(done As Integer, total As Integer) As Integer
    If total <= 0 Then
        Percent = 100
    Else
        Percent = (done * 100) / total
    End If
End Function

' Disegna una barra percentuale, per esempio l'energia del player.
Sub DrawMeter(x As Integer, y As Integer, w As Integer, h As Integer, value As Integer, color As Integer)
    Dim fillW As Integer

    If value < 0 Then value = 0
    If value > 100 Then value = 100
    fillW = (w * value) / 100

    FillRect x, y, x + w, y + h, Rgb(45, 48, 56)
    FillRect x, y, x + fillW, y + h, color
    Rect x, y, x + w, y + h, Rgb(155, 160, 170)
End Sub

' Overlay rosso progressivo durante morte e respawn.
Sub DrawDeathOverlay(viewW As Integer, viewH As Integer, strength As Integer, respawning As Integer)
    Dim edgeSize As Integer
    Dim color As Integer

    If strength <= 0 Then Exit Sub
    If strength > 100 Then strength = 100

    edgeSize = (viewH * strength) / 220
    color = Rgb(130 + (strength / 2), 0, 0)

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

' Pannello laterale: debug, energia, completamento item/nemici e comandi.
Sub DrawHud(viewW As Integer, viewH As Integer, hudX As Integer, energy As Integer, autosaveTimer As Double, autosaveSeconds As Double, saved As Integer, gameDone As Integer)
    Dim enemies As Integer
    Dim killed As Integer
    Dim killPct As Integer
    Dim itemPct As Integer
    Dim items As Integer
    Dim collected As Integer
    Dim autoRemain As Integer

    enemies = RayEnemyCount()
    killed = RayKilledEnemyCount()
    items = RayItemCount()
    collected = RayCollectedItemCount()
    killPct = Percent(killed, enemies)
    itemPct = Percent(collected, items)

    FillRect viewW, 0, viewW + 320, viewH, Rgb(16, 18, 22)
    TextOut hudX, 24, "nuBASIC WinRayCast", Rgb(235, 235, 235)
    TextOut hudX, 58, "Sprites: " + Str$(RaySpriteCount()), Rgb(185, 205, 230)
    TextOut hudX, 82, "Actors : " + Str$(RayActorCount()), Rgb(185, 205, 230)

    TextOut hudX, 124, "ENERGY " + Str$(energy) + "%", Rgb(225, 225, 225)
    DrawMeter hudX, 148, 220, 16, energy, Rgb(60, 190, 115)

    TextOut hudX, 188, "KILLS " + Str$(killed) + "/" + Str$(enemies) + " " + Str$(killPct) + "%", Rgb(230, 230, 230)
    TextOut hudX, 212, "ITEMS " + Str$(collected) + "/" + Str$(items) + " " + Str$(itemPct) + "%", Rgb(230, 230, 230)

    TextOut hudX, 254, "X " + Str$(RayPlayerX()), Rgb(225, 225, 225)
    TextOut hudX, 278, "Y " + Str$(RayPlayerY()), Rgb(225, 225, 225)
    TextOut hudX, 302, "Facing " + Str$(RayPlayerFacing()), Rgb(225, 225, 225)

    If saved = 1 Then
        autoRemain = autosaveSeconds - autosaveTimer
        If autoRemain < 0 Then autoRemain = 0
        TextOut hudX, 344, "Autosave in " + Str$(autoRemain) + "s", Rgb(170, 210, 255)
    Else
        TextOut hudX, 344, "Autosave pending", Rgb(170, 210, 255)
    End If

    If gameDone = 1 Then
        TextOut hudX, 392, "MISSION COMPLETE", Rgb(150, 230, 150)
    Else
        TextOut hudX, 392, "Goal: kill 100% enemies", Rgb(190, 195, 205)
    End If

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
    Dim ignored As Integer
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
    ' GAME SETTINGS - modifica questi valori per cambiare il comportamento
    ' della demo senza toccare il resto del codice.
    ' ------------------------------------------------------------------
    viewW = 1024              ' Larghezza viewport 3D; allineata al demo C++.
    viewH = 1024              ' Altezza viewport 3D; evita lo schiacciamento 4:3.
    hudX = viewW + 16         ' X iniziale del pannello HUD laterale.
    delta = 0.016             ' Simula ~60 FPS; usato da animazioni/porte/nemici.
    moveStep = 28             ' Avanzamento per frame quando premi avanti.
    backStep = 0 - moveStep   ' Avanzamento negativo per la retromarcia.
    turnStep = 3.0            ' Gradi di rotazione per frame.
    turnLeftStep = 0.0 - turnStep
    energy = 100.0            ' Energia iniziale del player.
    energyMoveCost = 0.08     ' Costo energia per frame quando ti muovi.
    energyShotCost = 0.25     ' Costo energia per colpo sparato.
    autosaveSeconds = 5.0     ' Intervallo tra checkpoint automatici.
    respawnSeconds = 3.25     ' Attesa prima del ripristino dall'autosave.
    deathOverlaySeconds = 2.0 ' Tempo per arrivare al rosso pieno.
    fireDamage = 35.0         ' Danno base dello sparo del player.
    fireRangeCells = 12.0     ' Distanza massima dello sparo in celle.
    fireFovDegrees = 28.0     ' Cono frontale del colpo.
    fireCooldownSeconds = 0.35 ' Tempo minimo tra due colpi.

    ' Virtual-key Windows usati da RayKeyDown().
    vkEscape = 27             ' Esc: uscita.
    vkLeft = 37               ' Freccia sinistra: gira a sinistra.
    vkUp = 38                 ' Freccia su: avanti.
    vkRight = 39              ' Freccia destra: gira a destra.
    vkDown = 40               ' Freccia giu': indietro.
    vkA = 65                  ' A: gira a sinistra.
    vkD = 68                  ' D: gira a destra.
    vkQ = 81                  ' Q: uscita.
    vkS = 83                  ' S: indietro.
    vkW = 87                  ' W: avanti.
    vkSpace = 32              ' Spazio: spara.
    vkCtrl = 17               ' Ctrl: spara.
    autosaveTimer = autosaveSeconds ' Forza il primo autosave subito dopo il load.

    scriptBase = Pwd$()
    If argc > 0 Then
        scriptBase = argv(0)
    End If
    If argc > 1 Then
        If argv(1) = "--once" Then once = 1
    End If

    ignored = RaySetBaseDir(scriptBase) ' Rende i path relativi indipendenti dalla cwd dell'IDE.
    world = "raycast_demo/worlds/demo.world.json" ' Progetto demo WinRayCast da caricare.

    ignored = RayInit(viewW, viewH) ' Crea la sessione raycast con la risoluzione scelta.
    If RayLoadProject(world) = 0 Then
        If RayLoadWorld(world) = 0 Then
            Print "Cannot load WinRayCast demo world from: "; scriptBase
            Main = 1
            Exit Function
        End If
    End If

    checkpointX = RayPlayerX() ' Primo checkpoint: posizione iniziale del mondo.
    checkpointY = RayPlayerY()
    checkpointFacing = RayPlayerFacing()
    checkpointSaved = 1

    Screen 1 ' Apre la finestra grafica nuBASIC.
    MoveWindow 40, 40, viewW + 320, viewH + 40 ' Viewport 3D + pannello HUD.
    ScreenLock ' Disabilita il redraw automatico: disegniamo noi ogni frame.

    shouldQuit = 0
    While shouldQuit = 0
        If RayKeyDown(vkEscape) = 1 Or RayKeyDown(vkQ) = 1 Then shouldQuit = 1

        moving = 0 ' Flag per applicare il costo energia solo se ci muoviamo.
        If shotCooldown > 0.0 Then shotCooldown = shotCooldown - delta ' Timer anti-spam dello sparo.
        If lifeState = 0 And gameDone = 0 Then
            ' Input player: movimento e rotazione sono demandati all'engine.
            If RayKeyDown(vkLeft) = 1 Or RayKeyDown(vkA) = 1 Then ignored = RayTurn(turnLeftStep)
            If RayKeyDown(vkRight) = 1 Or RayKeyDown(vkD) = 1 Then ignored = RayTurn(turnStep)
            If RayKeyDown(vkUp) = 1 Or RayKeyDown(vkW) = 1 Then
                ignored = RayMove(moveStep)
                moving = 1
            End If
            If RayKeyDown(vkDown) = 1 Or RayKeyDown(vkS) = 1 Then
                ignored = RayMove(backStep)
                moving = 1
            End If
            If shotCooldown <= 0.0 Then
                If RayKeyDown(vkSpace) = 1 Or RayKeyDown(vkCtrl) = 1 Then
                    ignored = RayDamageEnemy(fireDamage, fireRangeCells, fireFovDegrees) ' Colpisce nemici/oggetti nel cono frontale.
                    shotCooldown = fireCooldownSeconds
                    energy = energy - energyShotCost
                End If
            End If

            If moving = 1 Then energy = energy - energyMoveCost ' Niente consumo passivo: solo azioni/danno.
            If energy <= 0.0 Then
                energy = 0.0
                lifeState = 1 ' Stato morte: blocca input e mostra overlay rosso.
                deathTimer = 0.0
            End If

            autosaveTimer = autosaveTimer + delta
            If autosaveTimer >= autosaveSeconds Then
                checkpointX = RayPlayerX()
                checkpointY = RayPlayerY()
                checkpointFacing = RayPlayerFacing()
                checkpointSaved = 1
                autosaveTimer = 0.0
            End If
        Else
            deathTimer = deathTimer + delta
            If deathTimer >= respawnSeconds Then
                ignored = RaySetPlayer(checkpointX, checkpointY, checkpointFacing) ' Respawn dall'ultimo autosave.
                energy = 100.0
                lifeState = 0
                deathTimer = 0.0
                autosaveTimer = 0.0
            End If
        End If

        ignored = RayUpdate(delta) ' Avanza animazioni, porte, pickup, nemici, ascensori e musica.
        If lifeState = 0 And gameDone = 0 Then
            incomingDamage = RayConsumePlayerDamage() ' Gli attacchi nemici sono accumulati dall'engine.
            If incomingDamage > 0.0 Then energy = energy - incomingDamage
        End If
        ignored = RayRender(viewW, viewH) ' Renderizza nel framebuffer software.
        ignored = RayPresent(0, 0, viewW, viewH) ' Copia il framebuffer nella finestra grafica.

        enemies = RayEnemyCount()
        killed = RayKilledEnemyCount()
        If enemies > 0 And killed >= enemies Then gameDone = 1

        energyInt = energy
        DrawHud viewW, viewH, hudX, energyInt, autosaveTimer, autosaveSeconds, checkpointSaved, gameDone

        If lifeState <> 0 Then
            redStrength = (deathTimer * 100) / deathOverlaySeconds
            DrawDeathOverlay viewW, viewH, redStrength, 0
        End If
        If gameDone = 1 Then
            TextOut 410, 350, "MISSION COMPLETE", Rgb(160, 255, 160)
        End If

        Refresh ' Mostra tutto il frame disegnato sotto ScreenLock.
        If once = 1 Then shouldQuit = 1
        MDelay(16) ' Pausa breve per mantenere il ritmo ~60 FPS.
    Wend

    ScreenUnlock ' Riabilita il comportamento grafico standard di nuBASIC.
    Main = 0
End Function
