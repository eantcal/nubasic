#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  This program is a Breakout clone For nuBASIC (breakout3.bas)
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
'  Author: Antonino Calderone <acaldmail@gmail.com>
' -----------------------------------------------------------------------------

' Globals

Dim wall%(128)
Dim c%(8) 


' -----------------------------------------------------------------------------
' Setup Colors

c%(0) = Rgb( 0   , 0   , &hff )
c%(1) = Rgb( 0   , &hff, &hff ) 
c%(2) = Rgb( 0   , &hff, 0    )
c%(3) = Rgb( &hff, &hff, 0    )
c%(4) = Rgb( &hff, 0   , 0    )
c%(5) = Rgb( &hff, 0   , &hff )
c%(6) = Rgb( &hff, &hff, &hff )

bgColor% = 0
playerColor% = c%(1)


' -----------------------------------------------------------------------------
' Setup player keys
left_key$  = "n" : Rem move player left
right_key$ = "m" : Rem move player right
quit_key$  = "q" : Rem exit game

vkey_right% = 12
vkey_left%  = 10
' -----------------------------------------------------------------------------

MoveWindow GetWindowX(),GetWindowY(), 700, 600

Cls

Print "Welcome to nuBreakout3..."
Print
Print "To play press"
Print "  '"; left_key$; "' or LEFT ARROW to move left, "
Print "  '"; right_key$; "' or RIGHT ARROW to move right And "
Print "  '"; quit_key$ ; "' to quit" 

Delay 3

' -----------------------------------------------------------------------------
'  Main Loop
' -----------------------------------------------------------------------------

While 1
   restart_game% = 0
   totalscore% = 0 
   lives% = 3 
   level%=1 
   lines%=4
   columns% = 8

' -----------------------------------------------------------------------------
'  Level Loop
' -----------------------------------------------------------------------------

   While 1

      Randomize

      w% = 64
      h% = 40

      completed% = 0  

      a%=1 
      b%=1
      
      player% = w%/2-2 
      
      akey$=""
      
      x%=Int(w%*Rnd(1)/2)+5
      y%=lines%*2+1

      Cls
     
      FillRect 0,0, 700, 600, bgColor%
      

' ------------------------------------------------------------------------------

      InitWallMap lines%, columns%  
      DrawWall lines%, columns%
      MovePlayer akey$ 

' ------------------------------------------------------------------------------
'  Update Ball Position
' ------------------------------------------------------------------------------
      While 1 Do
         EraseBall x%, y%

         x%=x%+a%
         y%=y%+b%

         DrawBall x%,y%
         DrawWall lines%, columns%

' ------------------------------------------------------------------------------
'  Dispatch other events
' ------------------------------------------------------------------------------

         If x%<=2 or x%>w%-2 Then 
            a%=-a%
            beep 
         Else 
            If y%<=lines%*2 Then ProcessCollision lines%, columns%
         End If 

         Rem Checks for Game Over
         If completed%<0 Then
         
            l% = lines%*20
            TextOut 120, l%+40, "Game Over", c%(6)
            TextOut 120, l%+60, "Press any key to restart or "+ quit_key$ + " to QUIT", c%(6)
         
            a$=Input$(1)

            If a$=quit_key$ Then 
               Cls 
               Print "Good Bye !" 
               End
            End If
         
            restart_game% = 1
            Exit While
         End If

         If completed%>0 Then 
            Rem Level Completed

            totalscore%=totalscore%+(80*lines%) 
            level%=level%+1

            TextOut 150, lines%*20+40, "     Level Completed ",     c%(6)
            TextOut 150, lines%*20+40, "Press any key to continue", c%(6)
           
            a$=Input$(1)
            lives%=lives%+1 
            lines%=lines%+1 
           
            If lines%>9 Then lines%=9

            Exit While
         End If

         If y%>h% Then ProcessPlayerCollisions lines%, columns%
         
         vkey% = 0
         do 
            k% = GetVkey()
            if k%>0 then vkey% = k%
         loop while k%>0
         
         if vkey%<=0 then 
            akey$= ""
         elif vkey%=vkey_left% then 
            akey$ = "n"
         elseif vkey%=vkey_right% then 
            akey$ = "m"
         else
            akey$ = chr(vkey%)
         end if
 
         ' akey$=InKey$()
         If Len(akey$)>0 Then MovePlayer akey$
         
      End While

      If restart_game% Then Exit While

   End While : ' Level loop

End While : ' Game loop


' ------------------------------------------------------------------------------
Sub MovePlayer(thekey$)
' ------------------------------------------------------------------------------
   If thekey$=right_key$ Then player%=player%+1
   If thekey$=left_key$ Then player%=player%-1

   If thekey$=quit_key$ Then 
      Cls 
      Print "Good Bye !"
      End
   End If

   If player%<1 Then player%=1 Else If player%>w%-8 Then player%=w%-8

   DrawPlayer
End Sub


' ------------------------------------------------------------------------------
Sub DrawPlayer()
' ------------------------------------------------------------------------------
   FillRect 1+player%*10-10, (h%+2)*10, player%*10+90, (h%+2)*10+10, bgColor% 
   FillRect 1+player%*10, (h%+2)*10, player%*10+80, (h%+2)*10+10, playerColor%
End Sub


' ------------------------------------------------------------------------------
Sub DrawWall(lin%, col%)
' ------------------------------------------------------------------------------
   counter%=lines%*8

   For brickx%=0 to col%-1
      For bricky%=0 to lin%-1
         
         index% = brickx%+bricky%*col%

         bx%= brickx%*80 
         by%= bricky%*20

         If wall%(index%)=1 Then 
            bcolor%=c%(index% mod 7) 
         Else 
            bcolor%=0
            counter%=counter%-1
         End If
            
         FillRect bx%+1, by%+1, bx%+80+1, by%+20+1, bcolor% 
     
         DrawBall x%,y%

      Next bricky% 
   Next brickx%

   Rect 0, 0, 642, 480, 255

   If level%<=12 Then MDelay 60-level%*5
   If counter%<=0 Then completed%=1

   FillRect 10, (h%+2)*10+20, 250, (h%+2)*10+50, bgColor%
   
   score$ = "  Score: "+Str$(totalscore%+(col%*lin%-counter%)*10)
   info$  = "Level: "+Str$(level%)+"  Lives: "+ Str$(lives%) + score$

   TextOut 10,  (h%+2)*10+30, info$, &hff0000

End Sub


' ------------------------------------------------------------------------------
Sub InitWallMap(lin%, col%)
' ------------------------------------------------------------------------------

   For brickx%=0 to col%-1
      For bricky%=0 to lin%-1
         pres%=1
         If level%>2 And Rnd(1)<0.5 Then pres%=0
         wall%(brickx%+bricky%*8) = pres%
      Next bricky% 
   Next brickx%

End Sub 


' ------------------------------------------------------------------------------
Sub ProcessCollision(lin%, col%)
' ------------------------------------------------------------------------------
   brickx% = (x%-1)\col%
   bricky% = 0
   If brickx%<0 Then brickx% = 0

   If b%<0 Then 
     bricky% = (y%-2)\2 
   Else
     bricky% = (y%+2)\2 
   End If 

   If not(bricky%<0 or bricky%>lines%) Then 
      If wall%(brickx%+bricky%*col%)<>0 Then 
         b%=-b%
         wall%(brickx%+bricky%*col%) = 0
         beep
      End If
   End If

   If y%<=2 Then 

      b%=1 

      If y%>=h%-2 Then 
         b%=-1
         beep
      End If 

   End If
 
End Sub


' ------------------------------------------------------------------------------
Sub ProcessPlayerCollisions(lin%, col%)
' ------------------------------------------------------------------------------
   If y%>h% And x%>=player% And x%<=player%+8 Then 
      b%=-b% 
      beep 
      exit Sub 
   End If

   lives%=lives%-1
  
   beep
   beep

Repeat:
   a$=Inkey$()
   if len(a$)=0 then Goto Repeat 
   
   If a$=quit_key$ Then 
      End 
   End If
   
   b%=-1
   Cls
  
   DrawWall lin%, col%
   DrawPlayer 
   
   If lives%<1 Then 
      completed%=-1
   End If

End Sub


' ------------------------------------------------------------------------------
Sub DrawBall(posx%, posy%)
' ------------------------------------------------------------------------------
   fillellipse posx%*10, posy%*10, posx%*10+10, posy%*10+10, &h2f00ff 
End Sub


' ------------------------------------------------------------------------------
Sub EraseBall(posx%, posy%)
' ------------------------------------------------------------------------------
   FillRect posx%*10, posy%*10, posx%*10+10, posy%*10+10, bgColor% 
End Sub


' -----------------------------------------------------------------------------

