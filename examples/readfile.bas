#!/usr/local/bin/nubasic
REM Readfile.bas - Read a text file showing its content on the screen
REM This file is part of nuBASIC
 
Cls
 
Input "Name of file to read "; filename$
 
Open filename$ For Input Access Read AS #1
 
If FError(1) Then
   Print "Cannot open '"; filename$; "'"
   End
End IF

While Not(FError(1)) And Not(Eof(1))
   Input# 1, line$
   Print line$
End While
 
Close #1
