#!/usr/local/bin/nubasic
10 Rem breakout.bas : This file is part of nuBASIC 
20 Rem (c) acaldmail@gmail.com
30 Rem
40 Rem Init variables
50 Dim wall%(64)
60 a=Rnd(-1) : w%=64 : h%=20 : completed%=0 : lives%=10
70 lives%=10
80 a%=1 : b%=1 : player% = w%/2-2 : akey$=""
90 x%=Int(w%*Rnd(1)/2)+5
100 lines%=4: y%=lines%+1
110 Cls
120 GoSub 510 : Rem Rectangle
130 GoSub 780 : Rem Init wall map
140 GoSub 580 : Rem Draw wall
150 GoSub 440 : Rem Draw player
160 Rem Draw ball
170 Locate y%,x%:Print " "
180 x%=x%+a%
190 y%=y%+b%
200 Locate y%,x%:Print "*"
210 GoSub 580
220 Rem -------------- COLLISIONs ------------------------
230 If x%<=2 or x%>w%-2 Then 
240 a%=-a%
250 Beep 
260 ElseIf y%<=lines%+1 Then 
270 GoSub 850 : Rem Process collitions
280 End if
290 If completed%<0 Then 
300   GoTo 1120 : Rem Completed (loose)
310 End if
320 If completed%>0 Then 
330   GoTo 940 : Rem Completed (win)
340 End if
350 If y%>h% Then 
360 GoSub 990
370 End if
380 akey$=inkey$()
390 If len(akey$)>0 Then 
400   GoSub 440 : Rem Draw player
410 End if
420 GoTo 170
430 Rem
440 Rem Draw Player
450   If akey$="m" Then player%=player%+1
460   If akey$="n" Then player%=player%-1
470   If akey$="q" Then End
480   If player%<1 Then player%=1 Else If player%>w%-6 Then player%=w%-6
490   Locate h%+2, player% : Print " ====== "
500   Return
510 Rem Rectangle
520   Locate 1,1
530   Print "|";: For i%=1 to w%-2: Print "-";: Next : Print "|"
540   For i%=1 to h%: Print "|"; SPC(w%-2);"|":Next  
550   Locate h%+3,1: For i%=1 to w%: Print "-"; : Next
560   Locate h%+4,w%\2-20: Print "nuBreakout - acaldmail@gmail.com (c) 2014"
570   Return
580 Rem Wall
590   brick$="|______|"
600   counter%=32
610   For brickx%=0 TO 7
620   For bricky%=0 TO lines%-1
630   index% = brickx%+bricky%*8
640   Locate bricky%+1, brickx%*Len(brick$)+1 
650   If wall%(index%)=1 Then Print brick$: ELSE Print SPC(Len(brick$)): counter%=counter%-1
660   If brickx%=0 AND wall%(index%)=0 Then Locate bricky%+1, brickx%*Len(brick$)+1 : Print "|" 
670   If brickx%=7 AND wall%(index%)=0 Then Locate bricky%+1, brickx%*Len(brick$)+8 : Print "|" 
680   Next bricky% 
690   Next brickx%
700   MDelay 100
710   If counter%<=0 Then completed%=1
720   Locate 5, w%+2 : Print "Lives: "; lives%
730   Locate 7, w%+2 : Print "Score: "; (32-counter%)*10
740   Locate 10, w%+2 : Print "[n]-LEFT"
750   Locate 11, w%+2 : Print "[m]-RIGHT"
760   Locate 12, w%+2 : Print "[q]-QUIT"
770   Return
780 Rem Init wall map
790   For brickx%=0 TO 7
800   For bricky%=0 TO lines%-1
810     wall%(brickx%+bricky%*8) = 1
820   Next bricky% 
830   Next brickx%
840   Return 
850 Rem ---- COLLISIONs
860   brickx% = (x%-1)\8
870   If b%<0 Then bricky% = y%-2 Else bricky% = y%-1
880   If wall%(brickx%+bricky%*8)=0 Then GoTo 920
890   wall%(brickx%+bricky%*8) = 0
900   b%=-b%: Beep
910   Return
920   If y%<=2 or y%>h%-1 Then b%=-b%: Beep
930   Return
940 Rem ---- COMPLETED
950   Locate h%\2,w%\2-12:   Print "Congratulation you win !"
960   Locate h%\2+2,w%\2-12: Print "Press any key to continue"
970   a$=Input$(1)
980   GoTo 60
990 Rem --- PLAYER collision
1000  If y%>h% and x%>=player% and x%<=player%+8 Then b%=-b%: Beep : Return
1010  lives%=lives%-1
1020  Locate h%\2,w%\2-12:   Print "Ops... you have "; lives%; " lives"
1030  Locate h%\2+2,w%\2-12: Print "Press any key to continue"
1040  a$=InKey$()
1050  If a$="q" Then End Else if len(a$) = 0 then Goto 1040
1060  b%=-1: cls
1070  GoSub 510 : Rem Rectangle
1080  GoSub 580 : Rem Draw wall
1090  GoSub 480 : Rem Draw player
1100  If lives%<1 Then completed%=-1
1110  Return
1120 Rem ---- COMPLETED
1130   Locate h%\2,w%\2-12:   Print "   Sorry, You loose !"
1140   Locate h%\2+2,w%\2-12: Print "Press any key to continue"
1150   a$=InKey$()
1160   If a$="q" Then end Else goto 1150
1170   GoTo 60
1180 Rem Rectangle
1190   Locate 1,1
1200   Print "|";: For i%=1 to w%-2: Print "-";: Next : Print "|"
1210   For i%=1 to h%: Print "|";spc(w%-2);"|":Next  
1220   Return
