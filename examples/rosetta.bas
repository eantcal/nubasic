#!/usr/local/bin/nubasic
' Rosetta.bas - This file is part of nuBASIC
' See http://rosettacode.org/wiki/Mandelbrot_set

Cls
MoveWindow GetWindowX(), GetWindowY(), 800, 600
FillRect 0,0,800,600,0

Print "Started  at  "; SysTime$()
h% = SysHour()
m% = SysMin()
s% = SysSec()

For x0 = -2 TO 2 Step .013
   For y0 = -1.5 TO 1.5 Step .013
      x = 0
      y = 0
      iteration = 0
      maxIteration = 223

      Do
         xtemp = x*x-y*y+x0
         y = 2*x*y+y0
         x = xtemp
         ++iteration
      Loop While ((x*x+y*y<=4) And (iteration < maxIteration))

      If iteration <> maxIteration Then c = iteration Else c = 0
      d%=150: dx%=300 : dy%=300
      FillRect x0*d%+dx%, y0*d%+dy%, x0*d%+dx%+2, y0*d%+dy%+2, Int(c)*16
    Next y0
Next x0


Print "Completed at "; SysTime$()

s% = SysSec()-s%
m% = SysMin()-m%
h% = SysHour()-h%

Print "Elapsed seconds "; s%+m%*60+h%*3600
