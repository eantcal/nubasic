#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  This program is a Calculator sample for nuBASIC (calc.bas)
' -----------------------------------------------------------------------------
'
'  This file is part of nuBASIC
'  Author: Antonino Calderone <acaldmail@gmail.com>
' -----------------------------------------------------------------------------


' -----------------------------------------------------------------------------
' Keyboard keys
' -----------------------------------------------------------------------------

Dim key( 30 ) as String
Const CalcLineLen as Integer = 25 

' -----------------------------------------------------------------------------
' Memory
' -----------------------------------------------------------------------------
Dim Memory as String
Memory = "0"


' -----------------------------------------------------------------------------
' Stack implementation
' -----------------------------------------------------------------------------

Dim resStack( 10000 ) as String
Dim stackPtr as Integer

stackPtr = 0

' -----------------------------------------------------------------------------

Sub Push( element as String )
   If stackPtr<10000 Then 
      stackPtr=stackPtr+1
      resStack( stackPtr ) = element
   End If
End Sub


' -----------------------------------------------------------------------------

Function Pop() as String
   Pop = resStack( stackPtr )
   If stackPtr>0 Then stackPtr = stackPtr - 1 
End Function


' -----------------------------------------------------------------------------

Sub ClearStack()
   resStack( 0 ) = "0"
   stackPtr = 0
End Sub


' -----------------------------------------------------------------------------

Sub SetupKeys()
   key( 5 ) = "Sin"
   key( 6 ) = "Cos"
   key( 7 ) = "Tan"
   key( 8 ) = "Sqr"
   key( 9 ) = "^2"

   key( 0 ) = "(_)"
   key( 1 ) = "M+"
   key( 2 ) = "MR"
   key( 4 ) = "C"
   key( 3 ) = "MC"

   key( 10) = "7"
   key( 11) = "8"
   key( 12) = "9"
   key( 13) = "/"
   key( 14) = "Int"

   key( 15) = "4"
   key( 16) = "5"
   key( 17) = "6"
   key( 18) = "*"
   key( 19) = "1/x"

   key( 20) = "1"
   key( 21) = "2"
   key( 22) = "3"
   key( 23) = "-"
   key( 24) = "="

   key( 25) = "0"
   key( 26) = "."
   key( 28) = "+"
   key( 27) = "^"
   key( 29) = "<-"
End Sub


' -----------------------------------------------------------------------------

Sub DrawCalc(x as Integer, y as Integer, t as String, res as Integer)
   FillRect x, y, x+300, y+420, &H808050
   FillRect x+10, y+20, x+300-10, y+80, &Hc0c0c0

   TextOut x+10,y, "nuBASIC Calc 1.3.1 - Press 'q' to quit", &hffa030

   Dim sText as String
   sText = spc(CalcLineLen - Len(t))+t

   Dim i as Integer

   If res Then
      For i= 0 to Len(sText)
         TextOut x+i*8, y+30, SubStr$(sText, i, 1), 0
      Next i
   Else
      TextOut x+40, y+30, sText, 0
   End If

   If val(Memory)>0 Then
      TextOut x+10, y+24, "M", &hff
   End If

   Dim lx as Integer, ly as Integer
   Dim col as Integer, tcol as Integer
   Dim k as String

   Dim r as Integer
   Dim c as Integer

   For r = 0 to 5
      For c = 0 to 4
         lx = x+25+c*50
         ly = y+100+r*50 
         col = &hF0F0a0
         k=key(c+r*5)

         tcol = 0

         If (k>="0" and k<="9" and k<>"1/x") or k="." Then col = &ha0e0b0
         If left(k,1)="M" Then col = &hFFFF00

         If k="C" Then 
            col = &h2020ff : tcol = &hffffff
         ElIf k="=" Then 
            col = &h00ff00 
         ElIf k="<-" Then 
            col = &h00ffff
         ElIf k="+" or k="-" or k="/" or k="*" Then 
            col=&hffffff
         End If

         FillRect lx, ly, lx+45, ly+45, col
         TextOut x+30+c*50, y+115+r*50, k, tcol

      Next c
   Next r

End Sub


' -----------------------------------------------------------------------------

Function MouseToFn(x as Integer, y as Integer) as Integer
   Dim btn as Integer 
   Dim mx as Integer, my as Integer   
   Dim lx as Integer, ly as Integer  
   Dim r as Integer, c as Integer  

   mx = GetMouseX()
   my = GetMouseY()
   btn = GetMouseBtn()
   MouseToFn = -1

   For r = 0 to 5
      For c = 0 to 4

         lx = x+25+c*50
         ly = y+100+r*50

         If btn>0 and mx>=lx and mx<=lx+45 and my>=ly and my<=ly+45 Then
            MouseToFn = c+r*5
            MDelay 200
            Exit Function
         End If
      Next c
   Next r

End Function


' -----------------------------------------------------------------------------

Function addFunc( fn as Integer, text as String ) as String
   addFunc = ""
  
   Dim l as Integer
   l = Len(text)

   If fn=5 and (l+5) < CalcLineLen Then addFunc="sin(" + text + ")" : Exit Function
   If fn=6 and (l+5) < CalcLineLen Then addFunc="cos(" + text + ")" : Exit Function
   If fn=7 and (l+5) < CalcLineLen Then addFunc="tan(" + text + ")" : Exit Function
   If fn=8 and (l+5) < CalcLineLen Then addFunc="sqr(" + text + ")" : Exit Function
   If fn=9 and (l+4) < CalcLineLen Then addFunc="(" + text + ")^2"  : Exit Function
   If fn=0 and (l+2) < CalcLineLen Then addFunc="(" + text + ")"    : Exit Function

   If fn=14 and (l+5) < CalcLineLen Then addFunc="int(" + text + ")"  : Exit Function  
   
   If fn=4 and (l+2) < CalcLineLen Then addFunc="0" : Exit Function

   If text="0" Then text="" : l=l-1
   
   If fn=10 and (l+1) < CalcLineLen Then addFunc=text + "7" : Exit Function
   If fn=11 and (l+1) < CalcLineLen Then addFunc=text + "8" : Exit Function
   If fn=12 and (l+1) < CalcLineLen Then addFunc=text + "9" : Exit Function
   If fn=15 and (l+1) < CalcLineLen Then addFunc=text + "4" : Exit Function
   If fn=16 and (l+1) < CalcLineLen Then addFunc=text + "5" : Exit Function
   If fn=17 and (l+1) < CalcLineLen Then addFunc=text + "6" : Exit Function
   If fn=20 and (l+1) < CalcLineLen Then addFunc=text + "1" : Exit Function
   If fn=21 and (l+1) < CalcLineLen Then addFunc=text + "2" : Exit Function
   If fn=22 and (l+1) < CalcLineLen Then addFunc=text + "3" : Exit Function
   If fn=25 and (l+1) < CalcLineLen Then addFunc=text + "0" : Exit Function

   If text="" Then text="0" : l=l+1

   If fn=13 and (l+1) < CalcLineLen Then addFunc=text + "/" : Exit Function
   
   If fn=18 and (l+1) < CalcLineLen Then addFunc=text + "*" : Exit Function
   If fn=23 and (l+1) < CalcLineLen Then addFunc=text + "-" : Exit Function
   If fn=26 and (l+1) < CalcLineLen Then addFunc=text + "." : Exit Function
   If fn=28 and (l+1) < CalcLineLen Then addFunc=text + "+" : Exit Function

   If fn=27 and (l+3) < CalcLineLen Then addFunc="(" + text + ")^" : Exit Function
   If fn=19 and (l+4) < CalcLineLen Then addFunc="1/(" + text + ")": Exit Function

   addFunc = text
 
End Function


' -----------------------------------------------------------------------------

Sub Main()

   Cls
   MoveWindow GetWindowX(),GetWindowY(),320,500
   MDelay 300
   FillRect 0,0,GetSWidth(),GetSHeight(),0
   SetupKeys

   Dim Text as String

   Text = "0"
   DrawCalc 10,10, Text, 1

   Push "0"


' -----------------------------------------------------------------------------
' Main Loop
' -----------------------------------------------------------------------------
   Dim fn as Integer
   Dim l as Integer
   Dim c as String

   While true
      fn = MouseToFn(10,10)

      if InKey() = "q" then End

      If fn >= 0 Then 

         If fn=29 Then 
            Text = Pop()    
         Else
            If (fn<1 or fn>3) Then
               If Len(Text)>5 Then
                  If Right(Text,5)<>"Error" Then Push Text
               Else
                  Push Text
               End If
            End If

            If fn = 24 Then
               Text = str(eval(Text))
            Else
               Text = addFunc( fn, Text )
            End If

            If fn=3 or fn=4 Then 
               Memory = "0" 
               If fn=4 Then ClearStack
            ElIf fn=1 Then
               Memory = str(eval( Memory + " + (" + Text + ")" ))
            ElIf fn=2 Then

               l = Len(Memory)
               c = Right(Text,1)

               If c>="0" and c<="9" and (Len(Text)+1)< CalcLineLen Then 
                  Text=Memory
               Else
                  If (Len(Text)+l) <  CalcLineLen Then Text=Text+Memory
               End If

            End If
         End If

         DrawCalc 10,10, Text, fn = 24 or fn = 4
      End If
   Wend

End Sub


' -----------------------------------------------------------------------------
Main
' -----------------------------------------------------------------------------
