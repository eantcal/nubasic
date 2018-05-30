#!/usr/local/bin/nubasic
5   Rem line.bas - This file is part of nuBASIC
6   Rem
10  Dim mx%(100), my%(100)
20  h = 50 : p = PI()
30  kx = 90 
40  ky = 150
50  dx% = 10
60  dy% = 10
70  gh1 = Rnd(1) * 3
80  gh2 = Rnd(1) * 3
90  For x= 0 TO 9 
100 For y = 0 TO 9
110 z = h * (Sin(x / 10 * p * gh1) * Sin(y / 10 * p * gh2))
120 zx = 50 + x * 20 + y * 6: zy = 120 - y * 20 - z + x * 2
130 mx%(x+y*dx%) = zx 
140 my%(x+y*dx%) = zy
150 Next y
160 Next x
170 col = &h00ffffff
180 For x = 0 TO 8
190 For y = 0 TO 8
200 Line mx%(x+ y*dx%) + kx, my%(x+ y*dx%) + ky, mx%(x + 1+ y*dx%) + kx, my%(x + 1+ y*dx%) + ky, col
210 Line mx%(x+ y*dx%) + kx, my%(x+ y*dx%) + ky, mx%(x+ (y + 1)*dx%) + kx, my%(x+ (y + 1)*dx%) + ky, col
220 Next y
230 Line mx%(x+ 9*dx%) + kx, my%(x+ 9*dx%) + ky, mx%(x + 1* 9*dx%) + kx, my%(x + 1+ 9*dx%) + ky, col
240 Line mx%(9+ x*dx%) + kx, my%(9+ x*dx%) + ky, mx%(9+ (x + 1)*dx%) + kx, my%(9+ (x + 1)*dx%) + ky, col
250 Next x
260 Line mx%(0+ 0) + kx, my%(0+ 0) + ky, mx%(0+ 0) + kx, my%(0+ 0) + ky + 80, col
270 Line mx%(9+ 9*dx%) + kx, my%(9+ 9*dx%) + ky, mx%(9+ 9*dx%) + kx, my%(9+ 9*dx%) + ky + 80, col
280 Line mx%(9+ 0) + kx, my%(9+ 0) + ky, mx%(9+ 0) + kx, my%(9+ 0) + ky + 80, col
290 Line mx%(9+ 0) + kx, my%(9+ 0) + ky, mx%(9+ 0) + kx, my%(9+ 0) + ky + 80, col
300 Line mx%(0+ 0) + kx, my%(0+ 0) + ky + 80, mx%(9+ 0) + kx, my%(9+ 0) + ky + 80, col
310 Line mx%(9+ 0) + kx, my%(9+ 0) + ky + 80, mx%(9+ 9*dx%) + kx, my%(9+ 9*dx%) + ky + 80, col
320 For i%=1 to 500: kk$ = InKey$(): MDelay 1: IF kk$="q" OR kk$="Q" Then End
330 Next i%
340 For i%=1 to 80: Print Spc(80): Next i%
350 Cls
355 FillRect 0, 0, 500, 500, 0
360 Randomize
370 GoTo 70
