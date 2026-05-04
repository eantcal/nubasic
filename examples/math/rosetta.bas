' Rosetta.bas - This file is part of nuBASIC
' See http://rosettacode.org/wiki/Mandelbrot_set

Cls
MoveWindow GetWindowX(), GetWindowY(), 800, 600
FillRect 0,0,800,600,0

TextOut 4, 4, "Started at " + SysTime$(), Rgb(255,255,0)
Dim _t0 As DateTime
_t0 = GetDateTime()
h% = _t0.hour
m% = _t0.minute
s% = _t0.second

For x0 = -2 TO 2 Step .013
   ScreenLock
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
   ScreenUnlock
Next x0

Dim _t1 As DateTime
_t1 = GetDateTime()
s% = _t1.second - s%
m% = _t1.minute - m%
h% = _t1.hour - h%

TextOut 4, 22, "Completed at " + SysTime$(), Rgb(255,255,0)
TextOut 4, 40, "Elapsed sec: " + Str$(s%+m%*60+h%*3600), Rgb(255,255,0)
