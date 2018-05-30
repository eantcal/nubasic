#!/usr/local/bin/nubasic
' graph2.bas - This file is part of nuBASIC

dx%=GetSWidth()
dy%=GetSHeight()

Randomize

While 1
   x1%=Rnd(1)*dx%
   x2%=Rnd(1)*dx%
   y1%=Rnd(1)*dy%
   y2%=Rnd(1)*dy%

   t%=Int(Rnd(1)*6)
   c%=Int(Rnd(1)*&hffffff)

   If t%=1 then
      Line x1%,y1%,x2%,y2%,c%
   Elif t%=2 then
      Rect x1%,y1%,x2%,y2%,c%
   Elif t%=3 then
      FillRect x1%,y1%,x2%,y2%,c%
   Elif t%=4 then
      Ellipse x1%,y1%,x2%,y2%,c%
   Else
      TextOut x1%,y1%, "nuBASIC",c%
   End if

   k$=InKey$()
   If k$="q" Then End
End While
