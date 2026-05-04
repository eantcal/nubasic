#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  plane.bas
' -----------------------------------------------------------------------------
'
'  This file is part of nuBASIC
'  Author: Antonino Calderone <acaldmail@gmail.com>
' -----------------------------------------------------------------------------

Const SkyColor% = &h0CC483F
Const AutoRealign% = 50
Const XRange% = 100
Const yHQuote% = 0
Const yLQuote% = 190
Const SkyLineWidth% = 2940
Const SkyLineHeight% = 240
Const SunXPos%=280
Const SunYPos%=0
Const HelpTextXPos% = 240
Const HelpTextYPos% = 520
Const HelpText$="Move plane using keys a,z,m,n, q to quit"
Const SkyLineBmp$="skyline.bmp"
Const SunBmp$="sun.bmp"

Const Plane$="plane.bmp"
Const PlaneUp$="planeup.bmp"
Const PlaneDown$="planedown.bmp"

' Bitmap handles (loaded once, drawn many times)
Dim bmpSkyLine as Integer
Dim bmpSun as Integer
Dim bmpPlane as Integer
Dim bmpPlaneUp as Integer
Dim bmpPlaneDown as Integer

Const KeyRight$="m"
Const KeyLeft$="n"
Const KeyUp$="a"
Const KeyDown$="z"

Const QuitKey$="q"

' ------------------------------------------------------------------------------

Run
End

' -----------------------------------------------------------------------------

Sub DrawSky()
   Cls
   MoveWindow GetWindowX(), GetWindowY(), 800, 600
   FillRect 0,400,800,400,SkyColor%
   FillRect 0,400,800,800,0
End Sub


' ------------------------------------------------------------------------------

Sub DrawScene(x As Integer, x_plane As Integer, y_plane As Integer, bmpPlane_id As Integer)
    If x Mod 100 = 0 Then FillRect 0, 0, 800, 300, SkyColor%

   BitmapDraw bmpSkyLine, -x, SkyLineHeight%
   BitmapDraw bmpSkyLine, -x + SkyLineWidth%, SkyLineHeight%
   BitmapDraw bmpPlane_id, 80 + x_plane, y_plane
   BitmapDraw bmpSun, SunXPos%, SunYPos%

   TextOut HelpTextXPos%, HelpTextYPos%, HelpText$, rgb(255, 255, 255)

End Sub


' ------------------------------------------------------------------------------

Sub Run()
    ' Load all bitmaps into cache once
    bmpSkyLine = BitmapLoad(SkyLineBmp$)
    bmpSun = BitmapLoad(SunBmp$)
    bmpPlane = BitmapLoad(Plane$)
    bmpPlaneUp = BitmapLoad(PlaneUp$)
    bmpPlaneDown = BitmapLoad(PlaneDown$)

    DrawSky()
    Dim y As Integer
    Dim x As Integer
    Dim xp As Integer
    Dim auto_x As Integer
    Dim auto_realign As Integer
    Dim cur_bmp As Integer
    cur_bmp = bmpPlane

    While True

        For x = 0 To 2940

            a$ = InKey()

            If a$ = QuitKey$ Then Exit Sub

            If a$ = KeyDown$ And y < yLQuote% Then y = y + 1
            If a$ = KeyUp$ And y > yHQuote% Then y = y - 1
            If a$ = KeyRight$ And xp < XRange% Then xp = xp + 1
            If a$ = KeyLeft$ And xp > (-XRange%) Then xp = xp - 1

            If Len(a$) = 0 Then
                auto_x = auto_x + 1
                If auto_x > 2 * XRange% And xp > 0 Then
                    xp = xp - 1
                End If

                If auto_x > 2 * XRange% And xp < 0 Then
                    xp = xp + 1
                End If

                If xp = 0 Then
                    auto_x = 0
                End If
            End If

            If auto_realign > 0 Then

                auto_realign = auto_realign - 1

                If auto_realign = 0 Then
                    cur_bmp = bmpPlane
                End If

            End If

            If a$ = KeyUp$ Then
                cur_bmp = bmpPlaneUp
                auto_realign = AutoRealign%
            End If

            If a$ = KeyDown$ Then
                cur_bmp = bmpPlaneDown
                auto_realign = AutoRealign%
            End If

            ScreenLock
            DrawScene x, xp, y, cur_bmp
         ScreenUnlock
        Next x
   Wend

   ' Release cached bitmaps
   BitmapFree bmpSkyLine
   BitmapFree bmpSun
   BitmapFree bmpPlane
   BitmapFree bmpPlaneUp
   BitmapFree bmpPlaneDown

End Sub


