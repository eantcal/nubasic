#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  minehunter.bas for nuBASIC is a clone of a famous game :)
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
' -----------------------------------------------------------------------------
Dim bg_board$(10)
Dim fg_board$(10)
Dim color%   (10)

board_offset_x% = 0
board_offset_y% = 0
cell_size%      = 40
game_over%      = 0
mine_found%     = 0
scbx%           = 0
scby%           = 430

' REM Declare global colors (update by SetupColors)
white%   = 0
gray%    = 0
gray2%   = 0
gray3%   = 0
gray4%   = 0
blue%    = 0
yellow%  = 0
red%     = 0
black%   = 0
green%   = 0
green2%  = 0
cyan%    = 0
hotpink% = 0
blue2%   = 0

version$="1.04"
game_mode% = 2

randomize

' Program entry point
SetupGame
MainLoop

' -----------------------------------------------------------------------------
' Procedures
' -----------------------------------------------------------------------------

' -----------------------------------------------------------------------------
Sub PlayTheGame()
' -----------------------------------------------------------------------------
   btn% = GetMouseBtn()
   xm% = GetMouseX()
   ym% = GetMouseY()
   x% = xm% div cell_size%
   y% = ym% div cell_size%

   If btn%<>0 And x%>=0 And x%<=9 And y%>=0 And y%<=9 Then
      If btn%=1 Then
         DiscoverBoundaries x%, y%
      ElIf btn%=2 Or btn%=4 Then
            SetFlag x%, y% 
      End If

      If mine_found%=0 Then
         UpdateBoard board_offset_x%, board_offset_y%, cell_size%
         DrawScoreBoard scbx%, scby%, cell_size%
      End If

      If mine_found% Then
         ShowAllMines board_offset_x%, board_offset_y%, cell_size%
         game_over% = 1
      Else
         UpdateBoard board_offset_x%, board_offset_y%, cell_size%
      End If
   End If

   If btn%<>0 And xm%>=scbx%+270 And ym%>=scby% And xm%<=scbx%+370 And ym%<=scby%+100 Then
      ShowAllMines board_offset_x%, board_offset_y%, cell_size%
      game_over% = 1
   End If
   
End Sub


' -----------------------------------------------------------------------------
Sub MainLoop()
' -----------------------------------------------------------------------------
   While 1

      While Not(game_over%) 
         PlayTheGame
      Wend

      FillRect 130, 460, 310, 545, gray%
      FillRect 110, 440, 300, 540, yellow%
      Rect     110, 440, 300, 540, white%

      TextOut 135, 455, "Game over... ",      blue%
      TextOut 135, 475, "Would you like to ", blue%
      TextOut 135, 495, "play again? Y/N",    blue%

      key$ = ""
  
      While key$<>"y" And key$<>"Y"
         key$ = Inkey$()
         If key$="n" Or key$="N" Then 
            FillRect 0, 0, 600, 800, black%
            Cls  
            Print "Bye bye"
            End
         End If 
      Wend

      game_over% = 0
      mine_found% = 0
      game_mode% = 2
      SetupGame 
   Wend
End Sub



' -----------------------------------------------------------------------------
Sub SetupGame()
' -----------------------------------------------------------------------------
   Randomize
   SetupColors
   
   Cls
   Locate 50, 1

   MoveWindow GetWindowX(),GetWindowY(),500,600
   FillRect 0,0,600,800,black%

   CleanGameArea board_offset_x%, board_offset_y%, cell_size%
   DrawScoreBoard scbx%, scby%, cell_size%
   
   TextOut 50, 20, "Click to choose ", white%
   
   FillRect 40, 60, 200, 100, yellow%
   TextOut 50, 70,  "Beginner", red%

   FillRect 40, 120, 200, 160, yellow%
   TextOut 50, 130, "Expert", red%
   
   btn% = GetMouseBtn()
   game_mode% = 2

   While game_mode% > 1
      btn% = 0

      While btn% = 0 
         btn% = GetMouseBtn()
         x% = GetMouseX()
         y% = GetMouseY()
      Wend

      If x%>=40 And x%<=200 And y%>=60 And y%<=100 Then
         game_mode% = 0  
      End If

      If x%>=40 And x%<=200 And y%>=120 And y%<=160 Then
         game_mode% = 1  
      End If

   Wend 

   SetupMines
   CleanForground

   MDelay 200 : ' ms

   CleanGameArea board_offset_x%, board_offset_y%, cell_size%
   DrawScoreBoard scbx%, scby%, cell_size%

   UpdateBoard board_offset_x%, board_offset_y%, cell_size%   
End Sub


' -----------------------------------------------------------------------------
Sub SetupColors()
' -----------------------------------------------------------------------------
   white%   = Rgb( &hff, &hff, &hff )
   gray%    = Rgb( &h80, &h80, &h80 )
   gray2%   = Rgb( &hE0, &hE0, &hE0 )
   gray3%   = Rgb( &hA0, &hA0, &hA0 )
   gray4%   = Rgb( &h40, &h40, &h40 )
   blue%    = Rgb( 0   , 0   , &hff )
   yellow%  = Rgb( &hff, &hff, 0    )
   red%     = Rgb( &hff, 0   , 0    )
   black%   = Rgb( 0   , 0   , 0    )
   green%   = Rgb( 0   , &hff, 0    )
   green2%  = Rgb( 0   , &h80, &h40 )
   cyan%    = Rgb( 0   , &hff, &hff )
   hotpink% = Rgb( &hff, 0   , &hff )
   blue2%   = Rgb( &h80, &hC0, &hff )   

   color%(0) = black%
   color%(1) = blue%
   color%(2) = green%
   color%(3) = red%
   color%(4) = cyan%
   color%(5) = hotpink%
   color%(6) = yellow%
   color%(7) = green2%
   color%(8) = gray%
   color%(9) = white%
End Sub


' -----------------------------------------------------------------------------
Sub DrawGameAreaFrame( offsetx%, offsety%, width% )
' -----------------------------------------------------------------------------
   Rect offsetx%-1, offsety%-1, offsetx% + width% * 10+1, offsety% + width% * 10+1, gray%
End Sub


' -----------------------------------------------------------------------------
Sub CleanGameArea( offsetx%, offsety%, width% )
' -----------------------------------------------------------------------------
   FillRect offsetx%, offsety%, offsetx% + width% * 10, offsety% + width% * 10, black%
   DrawGameAreaFrame offsetx%, offsety%, width%
End Sub


' -----------------------------------------------------------------------------
Sub DrawScoreBoard( offsetx%, offsety%, width% )
' -----------------------------------------------------------------------------
   x% = offsetx% + width% * 10
   y% = offsety% 
   
   FillRect offsetx%, y%, x%, y% + 120, color%(1)
   TextOut offsetx% + 15, y% + 20,  "Mine Hunter " + version$ , white%
   
   On game_mode% Goto beginner, expert, none

   beginner:  mode$ = "Beginner" : Goto skip
   expert:    mode$ = "Expert"   : Goto skip
   none:      mode$ = ""
   skip:
   
   If len(mode$) Then
      FillRect offsetx% + 270, y% + 20, offsetx% +370, y% + 100, gray4%
      TextOut offsetx% + 290, y% + 40,  "Done", green%
   End If
   
   TextOut offsetx% + 15, y% + 70, "Level : " + mode$, cyan%

End Sub


' -----------------------------------------------------------------------------
Sub ShowAllMines( offsetx%, offsety%, width% )
' -----------------------------------------------------------------------------
   For y% = 0 to 9
      For x% = 0 to 9
         c$ = mid$(bg_board$(y%), x%+1, 1)
         
         If c$ = "." Then c% = 0 Else c% = Val(c$)
         If c$ = "*" Then c% = 0
         
         xx% = x% * width% + offsetx%
         yy% = y% * width% + offsety%

         If c$ = "X" Then 
            FillRect xx%, yy%, xx% + width%, yy% + width%, red%
            c$ = "*"
         Else
            If mid$(fg_board$(y%), x%+1, 1)="F" Then
               If c$ = "*" Then
                  FillRect xx%, yy%, xx% + width%, yy% + width%, green%
               Else
                  FillRect xx%, yy%, xx% + width%, yy% + width%, yellow%
               End If
            Else
               FillRect xx%, yy%, xx% + width%, yy% + width%, gray2%
            End If
         End If
         rect xx%, yy%, xx% + width%, yy% + width%, black%
         If c$<>"." Then
            TextOut xx%+width%\3+2, yy%+width%\3+2, c$, black%
            TextOut xx%+width%\3+1, yy%+width%\3+1, c$, color%(c%)  
            TextOut xx%+width%\3,   yy%+width%\3,   c$, color%(c%) 
         End If
      Next x%
   Next y%
End Sub


' -----------------------------------------------------------------------------
Sub UpdateBoard( offsetx%, offsety%, width% )
' -----------------------------------------------------------------------------
   For y% = 0 to 9
      For x% = 0 to 9
         c$ = SubStr$(fg_board$(y%), x%, 1)
        
         xx% = x% * width% + offsetx%
         yy% = y% * width% + offsety%

         If c$ = "F" Then 
            FillRect xx%, yy%, xx% + width%, yy% + width%, gray3%
         ElIf c$ = "*" Then
               FillRect xx%, yy%, xx% + width%, yy% + width%, red%
               mine_found% = 1
               bg_board$(y%) = PStr$(bg_board$(y%), x%, "X")
         Else
            If c$ = " " Or Val%(c$)>0 Then 
               FillRect xx%, yy%, xx% + width%, yy% + width%, blue2%
            Else
               FillRect xx%, yy%, xx% + width%, yy% + width%, gray%
            End If
         End If

         rect xx%, yy%, xx% + width%, yy% + width%, black% 
         If c$<>"." Then 
            c% = Val%(c$)
            TextOut xx%+width%\3+2, yy%+width%\3+2, c$, black%
            TextOut xx%+width%\3+1, yy%+width%\3+1, c$, color%(c%)  
            TextOut xx%+width%\3,   yy%+width%\3,   c$, color%(c%) 
         End If
      Next x%
   Next y%
End Sub


' -----------------------------------------------------------------------------
Sub CleanForground()
' -----------------------------------------------------------------------------
   For i% = 0 to 9
      fg_board$(i%) = ".........."
   Next i%
End Sub


' -----------------------------------------------------------------------------
function GetMinedRow$()
' -----------------------------------------------------------------------------
   
   minedcol% = Int(Rnd(1) * 10)
   r$ = ".........."

   r$ = PStr$(r$, minedcol%, "*")

   If game_mode% Then
      Do 
         minedcol2% = Int(Rnd(1) * 10)
      Loop While minedcol2% = minedcol%

      r$ = PStr$(r$, minedcol2%, "*")
   End If

   GetMinedRow$ = r$

End function


' -----------------------------------------------------------------------------
Sub FillNumbers(col%, row%)
' -----------------------------------------------------------------------------
   line_above$   = ".........." 
   line_current$ = bg_board$(row%)
   line_below$ =   ".........." 

   If row%>= 1 Then
      line_above$ = bg_board$(row%-1)
   End If

   If row%<9 Then
      line_below$ = bg_board$(row%+1)
   End If 

   For i% = 0 to 9
      If SubStr$(line_current$, i%, 1) = "*" Then

         If i%>0 Then
            c$ =SubStr$(line_current$, i%-1, 1)
            If c$<>"*" Then
               line_current$ = PStr$(line_current$, i%-1, str$(Val(c$)+1))
            End If
         End If 

         If i%<9 Then
            c$ =SubStr$(line_current$, i%+1, 1)
            If c$<>"*" Then
               line_current$ = PStr$(line_current$, i%+1, str$(Val(c$)+1))
            End If
         End If 


         If i%>0 Then
            c$ =SubStr$(line_above$, i%-1, 1)
            If c$<>"*" Then
               line_above$ = PStr$(line_above$, i%-1, str$(Val(c$)+1))
            End If
         End If 

         If i%<9 Then
            c$ =SubStr$(line_above$, i%+1, 1)
            If c$<>"*" Then
               line_above$ = PStr$(line_above$, i%+1, str$(Val(c$)+1))
            End If
         End If 

         c$ =SubStr$(line_above$, i%, 1)
         If c$<>"*" Then
            line_above$ = PStr$(line_above$, i%, str$(Val(c$)+1))
         End If

         If i%>0 Then
            c$ =SubStr$(line_below$, i%-1, 1)
            If c$<>"*" Then
                line_below$ = PStr$(line_below$, i%-1, str$(Val(c$)+1))
            End If
         End If 

         If i%<9 Then
            c$ =SubStr$(line_below$, i%+1, 1)
            If c$<>"*" Then
                line_below$ = PStr$(line_below$, i%+1, str$(Val(c$)+1))
            End If
         End If 

         c$ =SubStr$(line_below$, i%, 1)
         If c$<>"*" Then
            line_below$ = PStr$(line_below$, i%, str$(Val(c$)+1))
         End If

      End If

   Next i%

   If row%>0 Then bg_board$(row%-1) = line_above$
   If row%<9 Then bg_board$(row%+1) = line_below$
   bg_board$(row%) = line_current$
End Sub


' -----------------------------------------------------------------------------
Sub DiscoverBoundaries(col%, row%)
' -----------------------------------------------------------------------------
   cf$ = SubStr$( fg_board$(row%), col%, 1 )
   cb$ = SubStr$( bg_board$(row%), col%, 1 )

   If cf$="F" Or cf$=" " Or Val%(cf$)>0 Then exit Sub

   If cb$<>"." Then
      fg_board$(row%) = PStr$(fg_board$(row%), col%, cb$)
      exit Sub
   Else
      fg_board$(row%) = PStr$(fg_board$(row%), col%, " ")
   End If

   from_j% = col%-1
   to_j% = col%+1

   If to_j%>9 Then to_j%=9
   If from_j%<0 Then from_j%=0

   from_i% = row%-1
   to_i% = row%+1

   If to_i%>9 Then to_i%=9
   If from_i%<0 Then from_i%=0
   
   For i%=from_i% to to_i%
      For j%=from_j% to to_j%
         cf$ = SubStr$( fg_board$(i%), j%, 1 ) 
         If cf$ = "." Then
            DiscoverBoundaries j%, i% 
         End If
      Next j%
   Next i%

End Sub


' -----------------------------------------------------------------------------
Sub SetFlag(col%, row%)
' -----------------------------------------------------------------------------
   cf$ = SubStr$( fg_board$(row%), col%, 1 )
   If cf$="." Then
      fg_board$(row%) = PStr$(fg_board$(row%), col%, "F")
      MDelay 200
   Else 
      If cf$="F" Then fg_board$(row%) = PStr$(fg_board$(row%), col%, ".") : MDelay 200
   End If
End Sub


' -----------------------------------------------------------------------------
Sub SetupMines( )
' -----------------------------------------------------------------------------
   For i% = 0 to 9
      bg_board$(i%) = GetMinedRow$()
   Next i%
  
   For row% = 0 to 9
      For col% = 0 to 9
         FillNumbers col%, row%
      Next col%
   Next row%
End Sub

