#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  cartoon.bas
' -----------------------------------------------------------------------------
'
'  This file is part of nuBASIC
'  Author: Antonino Calderone <acaldmail@gmail.com>
' -----------------------------------------------------------------------------


MoveWindow GetWindowX(),GetWindowY(), 120, 200

Cls
FillRect 0,0,150,350,0

While 1
   
   For i% = 0 to 5

      PlotImage "walk.bmp",  -80*i%, 0

      FillRect 80 ,0, 400, 300, 0

      MDelay 100
      
   Next i%

Wend

