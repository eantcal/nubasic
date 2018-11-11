#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  bitmap.bas
' -----------------------------------------------------------------------------
'
'  This file is part of nuBASIC
'  Author: Antonino Calderone <acaldmail@gmail.com>

' ------------------------------------------------------------------------------
Input "Insert bitmap name: "; file$
' ------------------------------------------------------------------------------

FOpen file$, "r", 1

If FError(1) Then
   MsgBox "Error", file$ + " not found"
   End
End If

Close #1

PlotImage file$, 0, 0 

End
