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

Dim bmp as Integer
bmp = BitmapLoad(file$)
If bmp = 0 Then
   MsgBox "Error", "Cannot load " + file$
   End
End If
BitmapDraw bmp, 0, 0
BitmapFree bmp
