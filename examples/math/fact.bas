#!/usr/local/bin/nubasic
REM This file is part of nuBASIC - antonino.calderone@gmail.com
REM The following procedure uses recursion to 
REM calculate the factorial of its original argument.

Function fact%(n%)
  If n%<=1 Then
     fact%=1
  Else
     fact%=n%*fact%(n%-1)
  End If
End Function


Rem Main

Input "Insert a number ", n%
Print "Factorial of "; n%; " is "; fact%(n%)

