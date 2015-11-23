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


MoveWindow GetWindowX(),GetWindowY(), 500,500
FillRect 0,0,500,500,0
Cls

const ray%=150
const offset_x% = 100
const offset_y% = 100

const bgcolor% = rgb(64,64,128)
const hcolor%  = rgb(192,255,0)
const mcolor%  = rgb(192,128,0) 
const scolor%  = rgb(255,0,0)

const alpha_h = (PI() * 2.0) / 12.0
const alpha_m = (PI() * 2.0) / 60.0


sub DrawBg()
   dim i as Integer
   dim x as Double
   dim y as Double

   FillRect offset_x%-offset_x%/2, offset_x%*2/2, 500,50,0
   TextOut offset_x%*2-offset_x%/2, offset_y%/2, SysTime$(), rgb(255,255,255)

   FillEllipse offset_x%, offset_y%, offset_x%+2*ray%, offset_y%+2*ray%, bgcolor% 

   for i = 1 to 60 
      x = cos(alpha_m * i) * (ray% - 10)
      y = sin(alpha_m * i) * (ray% - 10)

      r% = ray% div 20

      px% = x + offset_x% + ray% - r% div 2
      py% = y + offset_y% + ray% - r% div 2


      FillEllipse px%, py%, px% + r%, py% + r%, mcolor%
   next i


   for i = 1 to 12 
      x = cos(alpha_h * i) * (ray% - 10)
      y = sin(alpha_h * i) * (ray% - 10)

      r% = ray% div 10

      px% = x + offset_x% + ray% - r% div 2
      py% = y + offset_y% + ray% - r% div 2


      FillEllipse px%, py%, px% + r%, py% + r%, hcolor%
   next i
end sub



Sub DrawHand(x60 as Double, rmin as Integer, t as Integer, l as integer, c as integer)

   x = cos(alpha_m * x60) * l
   y = sin(alpha_m * x60) * l

   r% = ray% div 20

   px% = x + offset_x% + ray% - r% div 2
   py% = y + offset_y% + ray% - r% div 2

   
   for i%=-t div 2 to t div 2
      xc = cos(alpha_m * (x60+i%)) * rmin
      yc = sin(alpha_m * (x60+i%)) * rmin

      r% = ray% div 20

      pcx% = xc + offset_x% + ray% - r% div 2
      pcy% = yc + offset_y% + ray% - r% div 2

      Line pcx%, pcy%, px% + r% div 2, py% + r% div 2, c
   next i%

   ' FillEllipse px%, py%, px% + r%, py% + r%, c
End Sub



sub DrawHands(hh as Double, mm as Double, ss as Double)
   
   dim x as Double
   dim y as Double

   dim m_alpha as Double

   m_alpha = mm * alpha_h/60

   hh=hh-3
   mm=mm-15

   DrawHand mm, 15, 5, (ray% - 30), mcolor%
   DrawHand ss, 15, 1, (ray% - 25), scolor%
   DrawHand hh*5+mm/12, 15, 7, (ray% - 60), hcolor% 
   
     
end sub


DrawBg
while (InKey$()<>"q")
  DrawHands SysHour(),SysMin(), SysSec()
  Sleep 1
  DrawBg
End While
DrawHands SysHour(),SysMin(), SysSec()
end
