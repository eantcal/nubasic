#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  This program is a Breakout clone for nuBASIC (breakout2.bas)
' -----------------------------------------------------------------------------
'
'  This file is part of nuBASIC
'  Author: Antonino Calderone <acaldmail@gmail.com>
' -----------------------------------------------------------------------------

'
Dim wall%(128)
Dim c%(8) 
vkey_left%=10
vkey_right%=12

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
Print " Use LEFT/RIGHT arrows to move, q to quit" 

Delay 3

Cls


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

      ScreenLock
      FillRect 0,0, 700, 500, 0
      GoSub InitWallMap
      GoSub DrawWall
      brickMoveCount%=0 : Rem Full wall drawn; periodic refresh every 10 moves if no hit
      GoSub HandlePlayer
      ScreenUnlock 


   ' -------------------------------------------------------------------------
    Rem Update Ball

      While 1 do
         ScreenLock
         FillRect x%*10, y%*10, x%*10+10, y%*10+10, 0
         x%=x%+a%
         y%=y%+b%
         brickMoveCount%=brickMoveCount%+1
      ' ----------- Dispatch event to handle ----------------------------------

         If x%<=2 Or x%>w%-2 Then
            a%=-a%
            Beep
         ElseIf y%<=lines%*2 Then
            dirty% = 0
            GoSub ProcessCollision
            If dirty% Then GoSub DrawWall : brickMoveCount%=0
         End If

         If brickMoveCount%>=50 Then GoSub DrawWall : brickMoveCount%=0

         FillEllipse x%*10, y%*10, x%*10+10, y%*10+10, &h2f00ff
         GoSub DrawHud
         ScreenUnlock

         Rem Pace like original (was ~60-level%*5 ms per frame when DrawWall ran every tick)
         fdelay% = 60 - level% * 5
         If fdelay% < 22 Then fdelay% = 22
         MDelay fdelay%

         If completed%<0 Then
            Rem Game Over

            TextOut 150, lines%*20+40, "     No more lives to spend...",        &hffffff
            TextOut 150, lines%*20+60, "Press any key to restart Or q to QUIT", &hffffff
            Refresh

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
            Refresh

            a$=input$(1)
            lives%=lives%+1 
            lines%=lines%+1 
           
            If lines%>9 Then lines%=9

            Exit While
         End If

         If y%>h% Then GoSub ProcessPlayerBallCollision
         
         vkey%=0
         Do
            tempvkey%=GetVKey()
            If tempvkey%>0 Then vkey%=tempvkey%
         Loop While tempvkey%>0

         akey$=InKey$()
         If vkey%>0 Or Len(akey$)>0 Then 
            GoSub HandlePlayer
         End If
      End While

      If restart_game% Then Exit While

   End While : ' Level loop

End While : ' Game loop

' -------------------------------------------------------------------------
HandlePlayer:
   If vkey%=vkey_right% Then player%=player%+1
   If vkey%=vkey_left% Then player%=player%-1
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
   Rem Redraw brick field only (ball is drawn separately — avoids flicker)
   For brickx%=0 to 7
      For bricky%=0 to lines%-1
         index% = brickx%+bricky%*8
         bx%= brickx%*80 : by%= bricky%*20
         If wall%(index%)=1 Then 
            bcolor%=c%(index% Mod 7) 
         Else 
            bcolor%=0
         End If
         FillRect bx%+1, by%+1, bx%+80+1, by%+20+1, bcolor% 
      Next bricky% 
   Next brickx%

   rect 0,0,642,480, 255

   Return


' -------------------------------------------------------------------------
DrawHud:
   Rem Score / level / win check without redrawing every brick each frame
   counter%=lines%*8
   For brickx%=0 to 7
      For bricky%=0 to lines%-1
         index% = brickx%+bricky%*8
         If wall%(index%)=0 Then counter%=counter%-1
      Next bricky% 
   Next brickx%

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
         dirty% = 1

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
  
   Do 
      a$=inkey$()
   Loop While len(a$)<1 
  
   If a$="q" Then End
   
   b%=-1
   Cls
  
   GoSub DrawWall   
   brickMoveCount%=0
   GoSub DrawPlayer 
   
   If lives%<1 Then 
      completed%=-1
   End If

   Return

 ' -------------------------------------------------------------------------

