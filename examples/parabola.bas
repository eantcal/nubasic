#!/usr/local/bin/nubasic
'  Parabola.bas - This file is part of nuBASIC

   Cls

   Rem Draw axis

   white% = Rgb(&hff, &hff, &hff)
   red%   = Rgb(&hff, 0   , 0   )

   xw% = GetSWidth()
   yw% = GetSHeight()

   FillRect 0,0, GetWindowDx(), GetWindowDy(), &h0401010

   Line xw% div 2, 0, xw% div 2, yw%,   red%
   Line 0, yw% div 2, xw%,   yw% div 2, red%

   old_x = 0
   old_y = 0
   x = -400

   zoomfact = 1/30
   
   While 1
      y=(x^2) * zoomfact  : Rem Curve law

      If x>xw% div 2 Then End

      y%=yw% div 2-y
      x=x+0.5

      If old_x <> 0 Then 
         Line old_x+xw% div 2, old_y, x+xw% div 2, y%, white%
      End If

      old_x = x
      old_y = y%
    
   End While
