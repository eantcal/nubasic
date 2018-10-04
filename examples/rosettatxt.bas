' RosettaTxt.bas - This file is part of nuBASIC
' See http://rosettacode.org/wiki/Mandelbrot_set

Cls

Print "Started  at  "; SysTime$()
h% = SysHour()
m% = SysMin()
s% = SysSec()

For x0 = -2 TO 2 Step 0.013
   For y0 = -1.5 TO 1.5 Step 0.013
      x = 0
      y = 0
      iteration = 0
      maxIteration=50

      Do
         xtemp = x*x-y*y+x0
         y = 2*x*y+y0
         x = xtemp
         ++iteration
      Loop While ((x*x+y*y<=4) And (iteration < maxIteration))

      If iteration <> maxIteration Then c = iteration Else c = 0
      wx%=30
      wy%=15
      dx%=80
      dy%=25

      Locate int(y0*wy%+dy%), int(x0*wx%+dx%)
      Print chr(c+33)
    Next y0
Next x0

Print "Completed at "; SysTime$()

s% = SysSec()-s%
m% = SysMin()-m%
h% = SysHour()-h%

Print "Elapsed seconds "; s%+m%*60+h%*3600
