#!/usr/local/bin/nubasic
' This file is part of nuBASIC

Recurs 4

' ----------------------------------------------------------------
Sub Recurs( x% )
   Print x%; " ";

   x%=x%+1
   
   If x%>10 Then End
   
   Recurs x%
End Sub
