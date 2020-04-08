' -----------------------------------------------------------------------------
'  This program is a Calculator sample for nuBASIC (calc.bas)
' -----------------------------------------------------------------------------
'
'  This file is part of nuBASIC
'  Author: Antonino Calderone <acaldmail@gmail.com>
' -----------------------------------------------------------------------------

cls

' PRINT/PRINT# accept prefix $u. if such prefix is specified
' any sequence of \u0000-\uFFFF will be printed using 
' the corrisponding unicode symbol, depending also on the console configuration
' (e.g. console font)

for j=0 to 15
   for i=0 to 15
      h$ = hex$(j) + hex$(i)
      print $u, "Unicode symbol 26" + h$ + "= '\u26" + h$ + "'"
   next i
next j


end