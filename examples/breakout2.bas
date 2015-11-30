#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  This program is a Breakout clone for nuBASIC (breakout2.bas)
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

'
Dim wall%(128)
Dim c%(8) 


' -------------------------------------------------------------------------
' Setup Colors

c%(0) = &hff0000 
c%(1) = &hffff00 
c%(2) = &h00ff00
c%(3) = &h00ffff
c%(4) = &h0000ff
c%(5) = &hff00ff
c%(6) = &hffffff


' -------------------------------------------------------------------------

Cls


' -------------------------------------------------------------------------

Locate 3, 10  
Print "                  Welcome to nuBreakout2..."

Locate 4, 10 
Print " To play press key n to move left Or key m move right, q to quit" 

Delay 3


' -------------------------------------------------------------------------

Rem Game Loop

While 1
   restart_game% = 0
   totalscore% = 0 
   lives% = 3 
   level%=1 
   lines%=4


' -------------------------------------------------------------------------
'  Level Loop

   While 1
      Randomize

      w%=64
      h%=40
      completed%=0  
      a%=1 
      b%=1
      player% = w%/2-2 
      akey$=""
      x%=Int(w%*Rnd(1)/2)+5
      y%=lines%*2+1

      Cls

      FillRect 0,0, 700, 500, 0


      ' -------------------------------------------------------------------------

      GoSub InitWallMap  
      GoSub DrawWall     
      GoSub HandlePlayer 


   ' -------------------------------------------------------------------------
    Rem Update Ball

      While 1 do
         FillRect x%*10, y%*10, x%*10+10, y%*10+10, 0 
         x%=x%+a%
         y%=y%+b%
         FillEllipse x%*10, y%*10, x%*10+10, y%*10+10, &h2f00ff 

         GoSub DrawWall


      ' ----------- Dispatch event to handle ----------------------------------

         If x%<=2 Or x%>w%-2 Then 
            a%=-a%
            Beep 
         ElseIf y%<=lines%*2 Then 
            GoSub ProcessCollision
         End If 

         If completed%<0 Then
            Rem Game Over

            TextOut 150, lines%*20+40, "     No more lives to spend...",        &hffffff
            TextOut 150, lines%*20+60, "Press any key to restart Or q to QUIT", &hffffff
         
            a$=input$(1)

            If a$="q" Then 
               Cls 
               Print "good bye !" 
               End
            End If
         
            restart_game% = 1
            Exit While
         End If

         If completed%>0 Then 
            Rem Level Completed

            totalscore%=totalscore%+(80*lines%) : level%=level%+1

            TextOut 150, lines%*20+40, "     Level Completed ",     &hffffff 
            TextOut 150, lines%*20+40, "Press any key to continue", &hffffff 
           
            a$=input$(1)
            lives%=lives%+1 
            lines%=lines%+1 
           
            If lines%>9 Then lines%=9

            Exit While
         End If

         If y%>h% Then GoSub ProcessPlayerBallCollision
         
         akey$=InKey$()
         If Len(akey$)>0 Then 
            GoSub HandlePlayer
         End If
      End While

      If restart_game% Then Exit While

   End While : ' Level loop

End While : ' Game loop

' -------------------------------------------------------------------------
HandlePlayer:
   If akey$="m" Then player%=player%+1
   If akey$="n" Then player%=player%-1
   If akey$="q" Then End
   If player%<1 Then player%=1 Else If player%>w%-8 Then player%=w%-8

   GoSub DrawPlayer

   Return 

' -------------------------------------------------------------------------
DrawPlayer:
   FillRect 1+player%*10-10, (h%+2)*10, player%*10+90, (h%+2)*10+10, 0 
   FillRect 1+player%*10, (h%+2)*10, player%*10+80, (h%+2)*10+10, &hFFFF00 
   Return


' -------------------------------------------------------------------------
DrawWall:
   counter%=lines%*8

   For brickx%=0 to 7
      For bricky%=0 to lines%-1
         index% = brickx%+bricky%*8
         bx%= brickx%*80 : by%= bricky%*20
         If wall%(index%)=1 Then 
            bcolor%=c%(index% Mod 7) 
         Else 
            bcolor%=0: counter%=counter%-1
         End If
         FillRect bx%+1, by%+1, bx%+80+1, by%+20+1, bcolor% 
         FillEllipse x%*10, y%*10, x%*10+10, y%*10+10, &h2f00ff 
      Next bricky% 
   Next brickx%

   rect 0,0,642,480, 255

   If level%<=12 Then mdelay 60-level%*5
   If counter%<=0 Then completed%=1

   FillRect 10, (h%+2)*10+20, 280, (h%+2)*10+50, &hffffff

   score$ = "  Score: "+str$(totalscore%+(8*lines%-counter%)*10)
   info$="Level: "+str$(level%)+"  Lives: "+ str$(lives%) + score$

   TextOut 10,  (h%+2)*10+30, info$, &hff0000

   Return


' -------------------------------------------------------------------------
InitWallMap:

   For brickx%=0 to 7
      For bricky%=0 to lines%-1
         pres%=1
         If level%>2 And Rnd(1)<0.5 Then pres%=0
         wall%(brickx%+bricky%*8) = pres%
      Next bricky% 
   Next brickx%

   Return 


' -------------------------------------------------------------------------
ProcessCollision:
   brickx% = (x%-1)\8
   If brickx%<0 Then brickx% = 0
   If b%<0 Then bricky% = (y%-2)\2 

   If Not(bricky%<0 Or bricky%>lines%) Then 
      If wall%(brickx%+bricky%*8)<>0 Then 
         b%=-b%
         wall%(brickx%+bricky%*8) = 0

         Beep
      End If
   End If

   If y%<=2 Then 

      b%=1 

      If y%>=h%-2 Then 
         b%=-1
         Beep
      End If 

   End If
 
   Return 


' ------------------------------------------------------------------------
ProcessPlayerBallCollision:
   If y%>h% And x%>=player% And x%<=player%+8 Then 
      b%=-b% 
      Beep 
      Return
   End If

   lives%=lives%-1
  
   Beep
   Beep
  
   a$=input$(1)
  
   If a$="q" Then End
   
   b%=-1
   Cls
  
   GoSub DrawWall   
   GoSub DrawPlayer 
   
   If lives%<1 Then 
      completed%=-1
   End If

   Return

 ' -------------------------------------------------------------------------

