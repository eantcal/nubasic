' -----------------------------------------------------------------------------
'  Clock.bas
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


' -----------------------------------------------------------------------------

Struct time_hhmmss
  hh as Double
  mm as Double
  ss as Double
End Struct


' -----------------------------------------------------------------------------

Const ray as Integer = 150
Const offset_x as Integer = 100
Const offset_y as Integer = 100

Const bgcolor as Integer = rgb(64,64,128)
Const hcolor as Integer  = rgb(192,255,0)
Const mcolor as Integer  = rgb(192,128,0) 
Const scolor as Integer  = rgb(255,0,0)

Const alpha_h as Double = (PI() * 2.0) / 12.0
Const alpha_m as Double = (PI() * 2.0) / 60.0


' -----------------------------------------------------------------------------

MoveWindow GetWindowX(),GetWindowY(), 500,500
FillRect 0,0,500,500,0
Cls

' Call Main procedure
Main  

' Exit program
End   


' -----------------------------------------------------------------------------

Function getTime() as time_hhmmss
  getTime.hh = SysHour()
  getTime.mm = SysMin()
  getTime.ss = SysSec()
end Function


' -----------------------------------------------------------------------------

Sub DrawBg()
   FillRect offset_x-offset_x/2, 20, 600,80,0
   TextOut offset_x*2-offset_x/2, offset_y/2, SysTime(), rgb(255,255,255)

   FillEllipse offset_x, offset_y, offset_x+2*ray, offset_y+2*ray, bgcolor 

   Dim x as Double
   Dim y as Double

   Dim i as Integer
   Dim r as Integer

   For i = 1 to 60 
      x = Cos(alpha_m * i) * (ray - 10)
      y = Sin(alpha_m * i) * (ray - 10)

      r = ray Div 20

      px = x + offset_x + ray - r div 2
      py = y + offset_y + ray - r div 2


      FillEllipse px, py, px + r, py + r, mcolor
   Next i


   For i = 1 to 12 
      x = Cos(alpha_h * i) * (ray - 10)
      y = Sin(alpha_h * i) * (ray - 10)

      r = ray Div 10

      px = x + offset_x + ray - r div 2
      py = y + offset_y + ray - r div 2


      FillEllipse px, py, px + r, py + r, hcolor
   Next i
End Sub


' -----------------------------------------------------------------------------

Sub DrawHand(x60 as Double, rmin as Integer, t as Integer, l as integer, c as integer)

   Dim r as Integer

   x = cos(alpha_m * x60) * l
   y = sin(alpha_m * x60) * l

   r = ray div 20

   px = x + offset_x + ray - r div 2
   py = y + offset_y + ray - r div 2


   Dim pcx as Double, pcy as Double
   Dim i as Integer

   for i=-t div 2 to t div 2
      xc = cos(alpha_m * (x60+i)) * rmin
      yc = sin(alpha_m * (x60+i)) * rmin

      r = ray div 20

      pcx = xc + offset_x + ray - r div 2
      pcy = yc + offset_y + ray - r div 2

      Line pcx, pcy, px + r div 2, py + r div 2, c
   next i

End Sub


' -----------------------------------------------------------------------------

sub DrawHands(t as time_hhmmss)
   dim x as Double
   dim y as Double
   dim m_alpha as Double, m12 as Double
   
   m_alpha = t.mm * alpha_h/60

   m12 = t.mm/12

   t.hh=t.hh-3
   t.mm=t.mm-15

   DrawHand t.mm, 15, 5, (ray - 30), mcolor
   DrawHand t.ss, 15, 1, (ray - 25), scolor
   DrawHand t.hh*5+m12, 15, 7, (ray - 60), hcolor  
end sub


' -----------------------------------------------------------------------------

sub Main()
   DrawBg

   while (InKey()<>"q")
     DrawHands getTime()
     Sleep 1
     DrawBg
   End While

   DrawHands getTime()
End Sub
