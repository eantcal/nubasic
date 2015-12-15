#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  plane.bas
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
Const HelpTextYPos% = 500
Const HelpText$="Move plane using keys a,z,m,n, q to quit"
Const SkyLineBmp$="skyline.bmp"
Const SunBmp$="sun.bmp"

Const Plane$="plane.bmp"
Const PlaneUp$="planeup.bmp"
Const PlaneDown$="planedown.bmp"

Const KeyRight$="m"
Const KeyLeft$="n"
Const KeyUp$="a"
Const KeyDown$="z"

Const QuitKey$="q"

' ------------------------------------------------------------------------------

Main
End

' -----------------------------------------------------------------------------

Sub DrawSky()
   Cls
   MoveWindow GetWindowX(), GetWindowY(), 800, 600
   FillRect 0,400,800,400,SkyColor%
   FillRect 0,400,800,800,0
End Sub


' ------------------------------------------------------------------------------

Sub DrawScene(x as Integer, x_plane as Integer, y_plane as Integer, pt as String)  
   If x mod 100 = 0 Then FillRect 0, 0, 800, 300, SkyColor%

   PlotImage SkyLineBmp$, -x,SkyLineHeight%
   PlotImage SkyLineBmp$, -x+SkyLineWidth%,SkyLineHeight%
   PlotImage pt, 80+x_plane,y_plane
   PlotImage SunBmp$, SunXPos%, SunYPos%

   TextOut HelpTextXPos%, HelpTextYPos%, HelpText$, rgb(255,255,255)
   
End Sub


' ------------------------------------------------------------------------------

Sub Main()
   DrawSky
   Dim y as Integer  
   Dim x as Integer
   Dim xp as Integer
   Dim auto_x as Integer
   Dim auto_realign as Integer
   Dim pt as String
   pt = "plane.bmp"

   While true

      For x = 0 to 2940

         a$=InKey()

         if a$=QuitKey$ then Exit Sub

         If a$=KeyDown$ And y<yLQuote% Then y=y+1
         If a$=KeyUp$ And y>yHQuote% Then y=y-1
         If a$=KeyRight$ And xp<XRange% Then xp=xp+1
         If a$=KeyLeft$ And xp>(-XRange%) Then xp=xp-1

         If Len(a$)=0 Then
            auto_x=auto_x+1
            If auto_x>2*XRange% And xp>0 Then 
               xp=xp-1
            End If
 
            If auto_x>2*XRange% And xp<0 Then
               xp=xp+1
            End If    

            If xp=0 Then
               auto_x = 0 
            End If
         End If
         
         If auto_realign>0 Then

            auto_realign=auto_realign-1

            If auto_realign=0 Then
               pt=Plane$
            End If

         End If
         

         If a$=KeyUp$ Then 
            pt = PlaneUp$
            auto_realign = AutoRealign%
         End If

         If a$=KeyDown$ Then 
            pt = PlaneDown$
            auto_realign = AutoRealign%
         End If
         
     
         DrawScene x, xp, y, pt
      Next x
   Wend
   
End Sub


