#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  cartoon.bas
' -----------------------------------------------------------------------------
'
'  This file is part of nuBASIC
'
'  nuBASIC is free software; you can redistribute it and/or modify
'  it under the terms of the GNU General Public License as published by
'  the Free Software Foundation; either version 2 of the License, or
'  (at your option) any later version.
'
'  nuBASIC is distributed in the hope that it will be useful,
'  but WITHOUT ANY WARRANTY; without even the implied warranty of
'  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'  GNU General Public License for more details.
'
'  You should have received a copy of the GNU General Public License
'  along with nuBASIC; if not, write to the Free Software
'  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
'
'  Author: <antonino.calderone@ericsson.com>, <acaldmail@gmail.com>
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

