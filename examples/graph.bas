' graph.bas - This file is part of nuBASIC

dx%=GetSWidth()
dy%=GetSHeight()

Init:
   x1%=Rnd(1)*dx%
   x2%=Rnd(1)*dx%
   y1%=Rnd(1)*dy%
   y2%=Rnd(1)*dy%

   t%=Int(Rnd(1)*6)
   c%=Int(Rnd(1)*&hffffff)

   On t% Goto _line, _rect, _frect, _ell, _fell, _text, skip

   _line:  Line x1%,y1%,x2%,y2%,c%        : Goto skip
   _rect:  Rect x1%,y1%,x2%,y2%,c%        : Goto skip
   _frect: FillRect x1%,y1%,x2%,y2%,c%    : Goto skip
   _ell:   Ellipse x1%,y1%,x2%,y2%,c%     : Goto skip
   _fell:  FillEllipse x1%,y1%,x2%,y2%,c% : Goto skip
   _text:  TextOut x1%,y1%, "nuBASIC",c%  : Goto skip

   skip:
      k$=InKey$()
      If k$="q" Then End

      Goto Init
