' -----------------------------------------------------------------------------
'  This program is a Calculator sample for nuBASIC (calc.bas)
' -----------------------------------------------------------------------------
'
'  This file is part of nuBASIC
'
'  nuBASIC is free software; you can redistribute it and/or modify
'  it under the terms of the GNU General Public License as published by
'  the Free Software Foundation; either version 2 of the License, or
'  (at your option) any later version.
'
'  nuBASIC is distributed in the hope that it will be useful,
'  but WITHOUT ANY WARRANTY; without even the implied warranty of
'  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'  GNU General Public License for more details.
'
'  You should have received a copy of the GNU General Public License
'  along with nuBASIC; if not, write to the Free Software
'  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
'
'  Author: <antonino.calderone@ericsson.com>, <acaldmail@gmail.com>
' -----------------------------------------------------------------------------


' -----------------------------------------------------------------------------
' Globals
' -----------------------------------------------------------------------------


Dim key$( 30 )
CalcLineLen% = 25

Dim resStack$( 10000 )
stackPtr% = 0

M$ = "0"

' -----------------------------------------------------------------------------

Sub Push( r$ )
   If stackPtr%<10000 Then 
      stackPtr%=stackPtr%+1
      resStack$( stackPtr% ) = r$
   End If
End Sub


' -----------------------------------------------------------------------------

Function Pop$()
   Pop$ = resStack$( stackPtr% )
   If stackPtr%>0 Then stackPtr% = stackPtr% - 1 
End Function


' -----------------------------------------------------------------------------

Sub ClearStack()
   resStack$( 0 ) = "0"
   stackPtr% = 0
End Sub


' -----------------------------------------------------------------------------

Sub SetupKeys()
   key$( 5 ) = "Sin"
   key$( 6 ) = "Cos"
   key$( 7 ) = "Tan"
   key$( 8 ) = "Sqr"
   key$( 9 ) = "^2"

   key$( 0 ) = "(_)"
   key$( 1 ) = "M+"
   key$( 2 ) = "MR"
   key$( 4 ) = "C"
   key$( 3 ) = "MC"

   key$( 10) = "7"
   key$( 11) = "8"
   key$( 12) = "9"
   key$( 13) = "/"
   key$( 14) = "Int"

   key$( 15) = "4"
   key$( 16) = "5"
   key$( 17) = "6"
   key$( 18) = "*"
   key$( 19) = "1/x"

   key$( 20) = "1"
   key$( 21) = "2"
   key$( 22) = "3"
   key$( 23) = "-"
   key$( 24) = "="

   key$( 25) = "0"
   key$( 26) = "."
   key$( 28) = "+"
   key$( 27) = "^"
   key$( 29) = "<-"
End Sub


' -----------------------------------------------------------------------------

Sub DrawCalc(x%, y%, t$, res%)
   FillRect x%, y%, x%+300, y%+420, &H808050
   FillRect x%+10, y%+20, x%+300-10, y%+80, &Hc0c0c0
   TextOut x%+10,y%, "nuBASIC Calc 1.3 - Press 'q' to quit application", &hffa030
   sText$ = spc(CalcLineLen% - Len(t$))+t$
   
   If res% Then
      For i%= 0 to Len(sText$)
         TextOut x%+i%*8, y%+30, SubStr$(sText$, i%, 1), 0
      Next i%
   Else
      TextOut x%+40, y%+30, sText$, 0
   End If

   If val(M$)>0 Then
      TextOut x%+10, y%+24, "M", &hff
   End If

   For r% = 0 to 5
      For c% = 0 to 4
         lx% = x%+25+c%*50
         ly% = y%+100+r%*50 
         col% = &hF0F0a0
         k$=key$(c%+r%*5)
         tcol% = 0
         If (k$>="0" and k$<="9" and k$<>"1/x") or k$="." Then col% = &ha0e0b0
         If left(k$,1)="M" Then col% = &hFFFF00

         If k$="C" Then 
            col% = &h2020ff : tcol% = &hffffff
         ElIf k$="=" Then 
            col% = &h00ff00 
         ElIf k$="<-" Then 
            col% = &h00ffff
         ElIf k$="+" or k$="-" or k$="/" or k$="*" Then 
            col%=&hffffff
         End If

         FillRect lx%, ly%, lx%+45, ly%+45, col%
         TextOut x%+30+c%*50, y%+115+r%*50, k$, tcol%
      Next c%
   Next r%

End Sub


' -----------------------------------------------------------------------------

Function MouseToFn%(x%, y%)
   
   mx% = GetMouseX()
   my% = GetMouseY()
   btn% = GetMouseBtn()
   MouseToFn% = -1

   For r% = 0 to 5
      For c% = 0 to 4

         lx% = x%+25+c%*50
         ly% = y%+100+r%*50

         If btn%>0 and mx%>=lx% and mx%<=lx%+45 and my%>=ly% and my%<=ly%+45 Then
            MouseToFn% = c%+r%*5
            MDelay 200
            Exit Function
         End If
      Next c%
   Next r%

End Function


' -----------------------------------------------------------------------------

Function addFunc$( fn%, t$ )
   addFunc$ = ""
   l% = Len(t$)

   If fn%=5 and (l%+5) < CalcLineLen% Then addFunc$="sin(" + t$ + ")" : Exit Function
   If fn%=6 and (l%+5) < CalcLineLen% Then addFunc$="cos(" + t$ + ")" : Exit Function
   If fn%=7 and (l%+5) < CalcLineLen% Then addFunc$="tan(" + t$ + ")" : Exit Function
   If fn%=8 and (l%+5) < CalcLineLen% Then addFunc$="sqr(" + t$ + ")" : Exit Function
   If fn%=9 and (l%+4) < CalcLineLen% Then addFunc$="(" + t$ + ")^2"  : Exit Function
   If fn%=0 and (l%+2) < CalcLineLen% Then addFunc$="(" + t$ + ")"    : Exit Function

   If fn%=14 and (l%+5) < CalcLineLen% Then addFunc$="int(" + t$ + ")"  : Exit Function  
   
   If fn%=4 and (l%+2) < CalcLineLen% Then addFunc$="0" : Exit Function

   If t$="0" Then t$="" : l%=l%-1
   
   If fn%=10 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "7" : Exit Function
   If fn%=11 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "8" : Exit Function
   If fn%=12 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "9" : Exit Function
   If fn%=15 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "4" : Exit Function
   If fn%=16 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "5" : Exit Function
   If fn%=17 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "6" : Exit Function
   If fn%=20 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "1" : Exit Function
   If fn%=21 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "2" : Exit Function
   If fn%=22 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "3" : Exit Function
   If fn%=25 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "0" : Exit Function

   If t$="" Then t$="0" : l% = l% + 1

   If fn%=13 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "/" : Exit Function
   
   If fn%=18 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "*" : Exit Function
   If fn%=23 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "-" : Exit Function
   If fn%=26 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "." : Exit Function
   If fn%=28 and (l%+1) < CalcLineLen% Then addFunc$=t$ + "+" : Exit Function

   If fn%=27 and (l%+3) < CalcLineLen% Then addFunc$="("+t$ + ")^" : Exit Function
   If fn%=19 and (l%+4) < CalcLineLen% Then addFunc$="1/("+t$ + ")": Exit Function

   addFunc$ = t$
 
End Function

' -----------------------------------------------------------------------------
' Program Entry Point
' -----------------------------------------------------------------------------

Cls
MoveWindow GetWindowX(),GetWindowY(),320,500
MDelay 300
FillRect 0,0,GetSWidth(),GetSHeight(),0
SetupKeys
Text$ = "0"
DrawCalc 10,10, Text$, 1
Push "0"


' -----------------------------------------------------------------------------
' Main Loop
' -----------------------------------------------------------------------------
While 1
   
   fn% = MouseToFn%(10,10)

   if InKey$() = "q" then End

   If fn% >= 0 Then 

      If fn%=29 Then 
         Text$ = Pop$()    
      Else
         If (fn%<1 or fn%>3) Then
            If Len(Text$)>5 Then
               If Right(Text$,5)<>"Error" Then Push Text$
            Else
               Push Text$
            End If
         End If

         If fn% = 24 Then
            Text$ = eval(Text$)
         Else
            Text$ = addFunc$( fn%, Text$ )            
         End If

         If fn%=3 or fn%=4 Then 
            M$ = "0" 
            If fn%=4 Then ClearStack
         ElIf fn%=1 Then
            M$ = str(val(eval( M$ + " + (" + Text$ + ")" )))
         ElIf fn%=2 Then
            l% = Len(M$)
            c$ = Right(Text$,1)
            If c$>="0" and c$<="9" and (Len(Text$)+1)< CalcLineLen% Then 
               Text$=M$
            Else
               If (Len(Text$)+l%) <  CalcLineLen% Then Text$=Text$+M$
            End If
         End If
      End If

      DrawCalc 10,10, Text$, fn% = 24 or fn% = 4
   End If

Wend
