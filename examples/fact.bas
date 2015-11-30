#!/usr/local/bin/nubasic
Rem This file is part of nuBASIC - acaldmail@gmail.com
Rem The following procedure uses recursion to 
Rem calculate the factorial of its original argument.

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

