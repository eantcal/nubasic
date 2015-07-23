' pingpong.bas - This file is part of nuBASIC

a% = 1
b% = 1

xw%=GetWindowX()
yw%=GetWindowY()
dx%=GetWindowDx()
dy%=GetWindowDy()

x% = xw% div 20
y% = yw% div 20

redraw% = 0

aLoop:
   x% = x% + a%
   y% = y% + b%

   if x%>=(dx% div 10) or x%<=0 then 
      xw% = GetWindowX() + (a% * 20)
      a% = -a%       
      redraw% = 1
   end if

   if y%>=(dy% div 10) or y%<=0 then 
      yw% = GetWindowY() + (b% * 20)
      b% = -b%     
      redraw% = 1
   end if

   if redraw%>0 then 
      redraw% = 0
      MoveWindow xw%,yw%,dx%,dy%
      beep
   end if


   fillellipse 10*x%, 10*y%, 10*x%+10, 10*y%+10, &hff
   mdelay 100
   
   fillellipse 10*x%, 10*y%, 10*x%+10, 10*y%+10, 0
   
   goto aLoop
