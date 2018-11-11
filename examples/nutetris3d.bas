#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  nuTetris3D.bas for nuBASIC is a clone of a famous game :)
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

MoveWindow GetWindowX(),GetWindowY(),640,600
MDelay 300

Dim piece$(30)
Dim piece_dx%(30)
Dim piece_dy%(30)
Dim bg_board$(30)
Dim fg_board$(30)
Dim preview_board$(4)
Dim color%(16)

board_offset_x% = 10
board_offset_y% = 10
atom_size% = 25
pw_atom_size% = 30
game_over% = 0
score% = 0
level% = 0

scbx% = 300
scby% = 20

version$="1.41"

cell_dx% = atom_size% * 2
cell_dy% = atom_size% * 2

camera_x% = 100
camera_y% = 30

visual_alpha% = 60
xres% = 180
ratio% = 1
yres% = 180
distance = 0
wall_type% = 0
kscale = 0.5
alpha_ratio = visual_alpha% / xres%

Dim first_h_int(2)
Dim first_v_int(2)
Dim h_int(2)
Dim v_int(2)

' -----------------------------------------------------------------------------
' Program entry point

piece_rnd% = Int(rnd(-1)) : REM randomize



DoRnd
SetupGame
MainLoop

' -----------------------------------------------------------------------------
' Procedures
' -----------------------------------------------------------------------------

' -----------------------------------------------------------------------------
Sub MainLoop()
' -----------------------------------------------------------------------------
   While 1

      While Not(game_over%) 
         MovePieces
      Wend

      FillRect 40, 60, 220, 160, &h808080
      FillRect 20, 40, 200, 140, 0
      Rect     20, 40, 200, 140, 255

      TextOut 50, 55, "Game over... ", &hffffff
      TextOut 50, 75, "Would you like to ", &hffffff
      TextOut 50, 95, "play again? Y/N", &hffffff
      key$ = ""
  
      While key$<>"y" and key$<>"Y"
         key$ = inkey$()
         If key$="n" Or key$="N" Then 
            FillRect 0,0,600,800,0
            Cls  
            Print "Bye bye"
            End
         End If 
      Wend

      game_over% = 0
      score% = 0
      CleanBackground
      CleanForground  
 
   Wend
End Sub


' -----------------------------------------------------------------------------
Sub SetupGame()
' -----------------------------------------------------------------------------
   SetupColors
   BuildPieces
   CleanBackground
   CleanForground   

   Cls
   FillRect 0,0,600,800,0
   DrawCleanGameArea board_offset_x%, board_offset_y%, atom_size%
   DrawScoreBoard scbx%, scby%, atom_size%
   
   
   TextOut 30, 40, "Welcome to nuTetris 3D " + version$, &hffffff
   
   TextOut 30, 70, "    To play press: ", &hffffff
   
   TextOut 30, 100, " 4        - Move left", &hffffff
   TextOut 30, 120, " 6        - Move right", &hffffff
   TextOut 30, 140, " 8 / 5    - Rotate", &hffffff
   TextOut 30, 160, " SPACEBAR - Drop ", &hffffff
    
   TextOut 30, 190, " (CTRL+C break the game)", &hffffff
   
   TextOut 30, 220, " PRESS ANY KEY TO PLAY", &hffffff
  
   Input$ 1 : ' Ignore return value

   DrawCleanGameArea board_offset_x%, board_offset_y%, atom_size%
   DrawScoreBoard scbx%, scby%, atom_size%
   
End Sub


' -----------------------------------------------------------------------------
Sub MovePieces()
' -----------------------------------------------------------------------------
   r% = 0
   x% = 3

   old_x% = 0
   old_y% = 0
   old_p% = 0
   old_r% = 0

   p% = piece_rnd%
   DoRnd

   render_on%=1
   no_more_space% = 0
   collision% = 0

   PutPreviewPiece piece_rnd%, 0
   TextOut 400, 135, "Next:", &hffffff

   DrawPreview 380, 160, pw_atom_size%

   For y% = 0 to 19
   
      For t% = 0 to 3
         k$ = InKey$()
         
         If k$="8" Then 
            rr% = r% + 1
            If rr% > 3 Then rr% = 0
            If Not(isPieceColliding%(x%,y%+4,p%, rr%)) Then 
               r%=rr%
            Else
               k$ = "4"
               Beep
            End If
         Else
            If k$="5" Then 
               rr% = r% - 1
               If rr%<0 Then rr%=3
               If Not(isPieceColliding%(x%,y%+4,p%,rr%)) Then 
                  r%=rr%
               Else
                  k$ = "4"
                  Beep
               End If
            End If
         End If 

         dx% = piece_dx%(p%*4+r%)

         If k$="4" Then 
           
           If x%>0 Then
              If Not(isPieceColliding%(x%-1,y%+4,p%,r%)) Then 
                 x%=x%-1
              End If
           End If

         Else

            If k$="6" Then 
               If (x%+1+dx%)<=10 Then
                  If Not(isPieceColliding%(x%+1,y%+4,p%,r%)) Then 
                     x%=x%+1
                  End If
               End If
            End If
 
         End If

         If k$=" " Then render_on% = 0

         collision% = 0
         If y%<=16 Then
           collision% = isPieceColliding%(x%,y%+4,p%,r%) 
           If collision% and y%<2 Then no_more_space% = 1
         End If
        
         If (y%>16) Or collision% Then
            PutPiece old_x%, old_y%, old_p%, old_r%, 1
            y%=19
            render_on%=1
            Beep 

            If PaintCompletedLines%() Then
               DrawBoard board_offset_x%, board_offset_y%, atom_size%
               mdelay 300
            End If

            If RemoveCompletedLines%() Then
               Beep
               DrawBoard board_offset_x%, board_offset_y%, atom_size%
               DrawGameAreaFrame board_offset_x%, board_offset_y%, atom_size%
               DrawScoreBoard scbx%, scby%, atom_size%
            End If

            collision% = 1
            Exit For 
         End If

         If t%=0 Or Len(k$)>0 Then
            CleanForground
            PutPiece x%,y%,p%,r%, 0
            old_x% = x%
            old_y% = y%
            old_p% = p%
            old_r% = r%
            
            If render_on% Or (y% Mod 5) = 0 Then
               DrawBoard board_offset_x%, board_offset_y%, atom_size%
            End If
            If render_on% Then
               DrawGameAreaFrame board_offset_x%, board_offset_y%, atom_size%
               DrawScoreBoard scbx%, scby%, atom_size%
            End If
         End If
         
         If Not(render_on%) Then Exit For      
      Next t%

     
      angle% = 240 + x%  
      camera_x% = cell_dx% * x% + 2*cell_dx%

      camera_y% = y%*cell_dy% + 4*cell_dy% 

      Move3DCamera angle% 
   Next y%

   If no_more_space% Then game_over% = 1
   
End Sub


' -----------------------------------------------------------------------------
Function isPieceColliding%( col%, row%, piece%, rot% )
' -----------------------------------------------------------------------------

   isPieceColliding% = 0   

   i%  = 0
   j%  = 0
   jj% = 0

   s$ = piece$( piece% * 4 + rot% )
   

   For jj%=0 to 3
      For i%=0 to 3
         
         j% = 3-jj%  

         idx% = (4 * j%) + i% + 1

         c$ = Mid(s$, idx%, 1)

         ridx% = row%+j%-4
         If ridx% >= 20 Or ridx%<0 Then Exit For

         b$ = Mid$(bg_board$(ridx%), i%+col%+1, 1)

         If c$<>"." and b$<>"." Then 
            isPieceColliding%=1 
            i% = 3
            Exit For 
         End If 
      Next i%
   Next jj%

End Function


' -----------------------------------------------------------------------------
Sub PutPiece( col%, row%, piece%, rot%, board_type% )
' -----------------------------------------------------------------------------
   
   For y% = 0 to 19

      row$ = ""

      For x% = 0 to 9

         c$ = "."

         If x%>=col% and x%<col%+4 and y%>=row% and y%<row%+4  Then
            i% = x%-col%
            j% = y%-row%
            s$ = piece$( piece% * 4 + rot% )
            c$ = Mid(s$, (4 * j%) + i% + 1, 1)
         End If

         If board_type% = 0 Then
            bc$ = Mid$(fg_board$(y%), x%+1, 1)
         Else
            bc$ = Mid$(bg_board$(y%), x%+1, 1)
         End If

         If bc$<>"." Then c$ = bc$

         row$ = row$ + c$

      Next x%

      If board_type% = 0 Then
         fg_board$(y%) = row$
      Else
         bg_board$(y%) = row$
      End If
      
    
   Next y%

End Sub


' -----------------------------------------------------------------------------
Sub PutPreviewPiece( piece%, rot% )
' -----------------------------------------------------------------------------
   
   For y% = 0 to 3

      row$ = ""

      For x% = 0 to 3

         s$ = piece$( piece% * 4 + rot% )
         c$ = Mid(s$, (4 * y%) + x% + 1, 1)
         row$ = row$ + c$

      Next x%

      preview_board$(y%) = row$   
    
   Next y%

End Sub


' -----------------------------------------------------------------------------
Function PaintCompletedLines%()
' -----------------------------------------------------------------------------
   PaintCompletedLines% = 0

   For y% = 19 to 0 Step -1

      completed% = 1

      For x% = 0 to 9
         bc$ = Mid$(bg_board$(y%), x%+1, 1)
         If bc$="." Then 
           completed%=0
           Exit For
         End If
      Next x%

      If completed%<>0 Then
         bg_board$(y%) = "9999999999"
         PaintCompletedLines% = 1
      End If

   Next y%


End Function


' -----------------------------------------------------------------------------
Function RemoveCompletedLines%()
' -----------------------------------------------------------------------------
   RemoveCompletedLines% = 0

   score_weight% = 1

   For t% = 0 to 3
      
      rowidx% = 0
       
      For y% = 19 to 0 Step -1

         completed% = 0

         bc$ = Mid$(bg_board$(y%), 1, 1)
         If bc$="9" Then 
            completed%=1
            rowidx% = y%
            Exit For
         End If

      Next y%

      If completed% Then
         While rowidx%>1 
            bg_board$(rowidx%) = bg_board$(rowidx%-1)
            rowidx%=rowidx%-1
         Wend

         score% = score% + score_weight%
         score_weight% = score_weight% + 1

         RemoveCompletedLines% = 1
      End If

   Next t%

End Function


' -----------------------------------------------------------------------------
Sub DrawGameAreaFrame( offsetx%, offsety%, width% )
' -----------------------------------------------------------------------------
   Rect offsetx%-1, offsety%-1, offsetx% + width% * 10+1, offsety% + width% * 20+1, color%(8)
End Sub


' -----------------------------------------------------------------------------
Sub DrawCleanGameArea( offsetx%, offsety%, width% )
' -----------------------------------------------------------------------------
   FillRect offsetx%, offsety%, offsetx% + width% * 10, offsety% + width% * 20, color%(0)
   DrawGameAreaFrame offsetx%, offsety%, width%
End Sub


' -----------------------------------------------------------------------------
Sub DrawScoreBoard( offsetx%, offsety%, width% )
' -----------------------------------------------------------------------------
   x% = offsetx% + width% * 10 + 20
   y% = offsety% 
   
   FillRect offsetx%, y%, x%, y% + 100, color%(0)
   Rect offsetx%, y%, x%, y% + 100, color%(2)
   TextOut offsetx% + 15, y% + 20,  "nuTetris 3D " + version$, color%(9) 
   TextOut offsetx% + 15, y% + 50,  "Score : " + str$( score% * 10 ), color%(3)
   level% = score% Div 50
   If level% > 9 Then level% = 9
   TextOut offsetx% + 15, y% + 70, "Level : " + str$( level%+1 ), color%(4)
   
End Sub


' -----------------------------------------------------------------------------
Sub DrawBoard( offsetx%, offsety%, width% )
' -----------------------------------------------------------------------------

   For y% = 0 to 19
      For x% = 0 to 9
         c$ = Mid$(fg_board$(y%), x%+1, 1)
         cs$ = Mid$(bg_board$(y%), x%+1, 1)
        
         If cs$ <> "." Then c$ = cs$

         If c$ = "." Then c% = 0 Else c% = Val(c$)
         
         xx% = x% * width% + offsetx%
         yy% = y% * width% + offsety%

         FillRect xx%, yy%, xx% + width%, yy% + width%, color%(c%)
         
         If (color%(c%)) > 0 Then
            Rect xx%, yy%, xx% + width%, yy% + width%, color%(8) 
         End If

      Next x%
   Next y%

End Sub


' -----------------------------------------------------------------------------
Sub DrawPreview( offsetx%, offsety%, width% )
' -----------------------------------------------------------------------------

   For y% = 0 to 3
      For x% = 0 to 3
         c$ = Mid$(preview_board$(y%), x%+1, 1) 
        
         If c$ = "." Then c% = 0 Else c% = Val(c$)
         
         xx% = x% * width% + offsetx%
         yy% = y% * width% + offsety%

         FillRect xx%, yy%, xx% + width%, yy% + width%, color%(c%)
         
         If (color%(c%)) > 0 Then
            Rect xx%, yy%, xx% + width%, yy% + width%, color%(8) 
         End If

      Next x%
   Next y%

End Sub


' -----------------------------------------------------------------------------
Sub CleanForground()
' -----------------------------------------------------------------------------
   For i% = 0 to 19
      fg_board$(i%) = ".........."
   Next i%
End Sub


' -----------------------------------------------------------------------------
Sub CleanBackground( )
' -----------------------------------------------------------------------------
   For i% = 0 to 19
      bg_board$(i%) = ".........."
   Next i%
End Sub


' -----------------------------------------------------------------------------
Sub BuildPieces()
' -----------------------------------------------------------------------------
   
   
   ' ....
   ' ....
   ' 11..
   ' 11..

   piece$(0) = "...." + "...." + "11.." + "11.."

   piece$(1) = piece$(0)
   piece$(2) = piece$(1)
   piece$(3) = piece$(2)

   piece_dx%(0) = 2
   piece_dx%(1) = 2
   piece_dx%(2) = 2
   piece_dx%(3) = 2

   piece_dy%(0) = 2
   piece_dy%(1) = 2
   piece_dy%(2) = 2
   piece_dy%(3) = 2


   ' 2...
   ' 2...
   ' 2...
   ' 2...
   
   piece$(4) = "2..." + "2..." + "2..." + "2..."

   piece$(6) = piece$(4)

   piece_dx%(4) = 1
   piece_dx%(6) = 1
   piece_dy%(4) = 4
   piece_dy%(6) = 4


   ' ....
   ' ....
   ' ....
   ' 2222

   piece$(5) = "...." + "...." + "...." + "2222"

   piece$(7) = piece$(5)

   piece_dx%(5) = 4
   piece_dx%(7) = 4   
   piece_dy%(5) = 1
   piece_dy%(7) = 1


   ' ....
   ' 3...
   ' 33..
   ' .3..
 
   piece$(8)  = "...." + "3..." + "33.." + ".3.."
   piece$(10) = piece$(8)

   piece_dx%(8) = 2
   piece_dx%(10) = 2
   piece_dy%(8) = 3
   piece_dy%(10) = 3

  
   ' ....
   ' ....
   ' .33.
   ' 33..

   piece$(9)  = "...." + "...." + ".33." + "33.."
   piece$(11) = piece$(9)

   piece_dx%(9)  = 3
   piece_dx%(11) = 3
   piece_dy%(9)  = 2
   piece_dy%(11) = 2


   ' ....
   ' .4..
   ' 44..
   ' 4... 

   piece$(12) = "...." + ".4.." + "44.." + "4..."
   piece$(14) = piece$(12)

   piece_dx%(12) = 2
   piece_dx%(14) = 2
   piece_dy%(12) = 3
   piece_dy%(14) = 3


   ' ....
   ' ....
   ' 44..
   ' .44.

   piece$(13) = "...." + "...." + "44.." + ".44."
   piece$(15) = piece$(13)


   piece_dx%(13) = 3
   piece_dx%(15) = 3
   piece_dy%(13) = 2
   piece_dy%(15) = 2
 

   ' ....
   ' 5...
   ' 55..
   ' 5...

   piece$(16) = "...." + "5..." + "55.." + "5..." 

   piece_dx%(16) = 2
   piece_dy%(16) = 3


   ' ....
   ' ....
   ' .5..
   ' 555.

   piece$(17) = "...." + "...." + ".5.." + "555."

   piece_dx%(17) = 3
   piece_dy%(17) = 3

   ' ....
   ' .5..
   ' 55..
   ' .5..

   piece$(18) = "...." + ".5.." + "55.." + ".5.." 

   piece_dx%(18) = 2
   piece_dy%(18) = 3

   ' ....
   ' ....
   ' 555.
   ' .5..

   piece$(19) = "...." + "...." + "555." + ".5.."

   piece_dx%(19) = 3
   piece_dy%(19) = 2

   ' ....
   ' 66..
   ' 6...
   ' 6...
   
   piece$(20) = "...." + "66.." + "6..." + "6..."

   piece_dx%(20) = 2
   piece_dy%(20) = 3


   ' ....
   ' ....
   ' 6...
   ' 666. 

   piece$(21) = "...." + "...." + "6..." + "666."

   piece_dx%(21) = 3
   piece_dy%(21) = 2


   ' ....
   ' .6..
   ' .6..
   ' 66.. 

   piece$(22) = "...." + ".6.." + ".6.." + "66.."

   piece_dx%(22) = 2
   piece_dy%(22) = 3


   ' ....
   ' ....
   ' 666.
   ' ..6. 

   piece$(23) = "...." + "...." + "666." + "..6."

   piece_dx%(23) = 3
   piece_dy%(23) = 2


   ' ....
   ' 77..
   ' .7..
   ' .7..
   
   piece$(24) = "...." + "77.." + ".7.." + ".7.."

   piece_dx%(24) = 2
   piece_dy%(24) = 3


   ' ....
   ' ....
   ' 777.
   ' 7... 

   piece$(25) = "...." + "...." + "777." + "7..."

   piece_dx%(25) = 3
   piece_dy%(25) = 2


   ' ....
   ' 7...
   ' 7...
   ' 77.. 

   piece$(26) = "...." + "7..." + "7..." + "77.."

   piece_dx%(26) = 2
   piece_dy%(26) = 3

   ' ....
   ' ....
   ' ..7.
   ' 777. 

   piece$(27) = "...." + "...." + "..7." + "777."

   piece_dx%(27) = 3
   piece_dy%(27) = 2

End Sub


' -----------------------------------------------------------------------------
Sub Move3DCamera(a%)
' -----------------------------------------------------------------------------
   a% = wrap_angle%(a%)
   ap% = wrap_angle%(a% + 30)
   a = (ap%/180)*PI()

   Textout 330, 310, "3D Camera", &hffffff
   Draw3DView 330, 350, yres%, Int(a%/(alpha_ratio)), ratio%
   Rect 329, 349, 332+xres% * ratio%, 352+yres%, &hffffff
End Sub


' -----------------------------------------------------------------------------
Sub DoRnd()
' -----------------------------------------------------------------------------
   piece_rnd% = Int(rnd(1)*1000) Mod 7
   If piece_rnd%>6 Then piece_rnd%=6
End Sub


' -----------------------------------------------------------------------------
Sub SetupColors()
' -----------------------------------------------------------------------------
   color%(0) = 0
   color%(1) = &hff0000
   color%(2) = &h00ff00
   color%(3) = &h0000ff
   color%(4) = &hffff00
   color%(5) = &hff00ff
   color%(6) = &h00ffff
   color%(7) = &h408000
   color%(8) = &h808080
   color%(9) = &hffffff
End Sub


' -----------------------------------------------------------------------------
Sub DrawMap2D( ox%, oy% )
' -----------------------------------------------------------------------------
   scale% = 5
   For y% = 0 to 19
      For x% = 0 to 9
         c$ = Mid$(bgboard$(y%), x%+1, 1) 
        
         c% = Val(c$)
      
         xx% = x% * scale% + ox%
         yy% = y% * scale% + oy%

         FillRect xx%, yy%, xx% + scale%, yy% + scale%, color%(c%)

      Next x%
   Next y%

   px% = Int(scale% * camera_x% / cell_dx%)
   py% = Int(scale% * camera_y% / cell_dy%)

   fillellipse ox% + px%-3, oy% + py%-3, ox% + px%+3, oy% + py%+3, &hff

End Sub


' -----------------------------------------------------------------------------
Sub FirstHorzIntersection(valpha%, M1, cell_x_pos%, cell_y_pos%) 
' -----------------------------------------------------------------------------
  ' yi
  If Not(valpha% >= 180 and valpha% < 360) Then 
     first_h_int(1) = cell_y_pos% * cell_dy%
  Else   
     first_h_int(1) = (cell_y_pos%+1) * cell_dy%   
  End If

  yp = camera_y%
  xp = camera_x%
  first_h_int(0) = M1 * (first_h_int(1) - yp) + xp

End Sub


' -----------------------------------------------------------------------------
Sub FirstVertIntersection(valpha%, M, cell_x_pos%, cell_y_pos%) 
' -----------------------------------------------------------------------------
  ' xi
   If valpha% >= 90 and valpha% < 270 Then 
      first_v_int(0) = (cell_x_pos%+1) * cell_dx%             
   Else
      first_v_int(0) = (cell_x_pos%) * cell_dx%             
   End If

  ' yi
   yp = camera_y%
   xp = camera_x%
   first_v_int(1) = M * (first_v_int(0) - xp) + yp   

End Sub


' -----------------------------------------------------------------------------
Sub VertIntersection(x,y, valpha%, M)
' -----------------------------------------------------------------------------
   If Not(valpha% >= 90 and valpha% < 270) Then
      v_int(0) = x - cell_dx%      : REM xi
      v_int(1) = y - M * cell_dy%  : REM yi
   Else 
      v_int(0) = x + cell_dx%      : REM xi
      v_int(1) = y + M * cell_dy%  : REM yi
   End If
End Sub


' -----------------------------------------------------------------------------
Sub HorzIntersection(x, y, valpha%, M1)
' -----------------------------------------------------------------------------
   If (valpha% >= 0 and valpha% < 180) Then
      h_int(0) = x - M1 * cell_dx% : REM xi
      h_int(1) = y - cell_dy%      : REM yi
   Else 
      h_int(0) = x + M1 * cell_dx% : REM xi
      h_int(1) = y + cell_dy%      : REM yi
   End If
End Sub


' -----------------------------------------------------------------------------
Function GetWallKey%( a%, x%, y% )
' -----------------------------------------------------------------------------
       
   c% = Int(x% Div cell_dx%)
   r% = Int(y% Div cell_dy%)

   GetWallKey% = GetKey%(c%, r%)
End Function


' -----------------------------------------------------------------------------
Function GetKey%(c%, r%)
' -----------------------------------------------------------------------------
   If c%>= 10 Then 
      c%=10
   Else
      If c%<(-1) Then c%=-1
   End If 

   If r%>= 20 Then
      r%=20
   Else
      If r%<0 Then r%=0
   End If

   If c%<0 Or c%>9 Or r%>19 Then
      GetKey% = 8
   Else
     row$ = bg_board$(r%)
     GetKey% = Val%( Mid$( row$, c%+1, 1 ) )
   End If
End Function


' -----------------------------------------------------------------------------
Function GetVertWallKey%( valpha%, x, y )
' -----------------------------------------------------------------------------
   c% = Int(x / cell_dx%)
   r% = Int(y / cell_dy%)
   
   If valpha% > 270 Or valpha% < 90 Then c% = c% - 1

   GetVertWallKey% = GetKey%(c%, r%)
   
End Function


' -----------------------------------------------------------------------------
Function GetHorzWallKey%( valpha%, x, y )
' -----------------------------------------------------------------------------
   c% = Int(x / cell_dx%)
   r% = Int(y / cell_dy%)

   If (valpha% >= 0 and valpha%< 180) Then r% = r% - 1

   GetHorzWallKey% = GetKey%(c%, r%)
   
End Function

   
' -----------------------------------------------------------------------------
Function ComputeDistance%(alpha, valpha%, xh, yh, xv, yv)
' -----------------------------------------------------------------------------
   dv = Abs((camera_x%-xv) / (Cos(alpha)+0.000001 )) 
   dh = Abs((camera_y%-yh) / (Sin(alpha)+0.000001 ))

   If dh<dv Then 
      distance = dh 
      ComputeDistance% = 1
   Else 
      distance = dv
      ComputeDistance% = 2
   End If
   
End Function


' -----------------------------------------------------------------------------
Function wrap_angle%(a%)
' -----------------------------------------------------------------------------
   If a%<0 Then
      wrap_angle% = 360 + a% 
   Else
      If a%>=360 Then 
         wrap_angle%=a%-360
      Else 
         wrap_angle% = a%
      End If
   End If


End Function


' -----------------------------------------------------------------------------
Sub Draw3DView( offsetx%, offsety%, dy%, ray%, fact% )
' -----------------------------------------------------------------------------  
   M1 = 0
   M = 0

   proj_angle% = wrap_angle%(Int( (ray%+(xres% Div 2)) * alpha_ratio))


   For i% = ray% to ray% +(xres%-1)

      valpha% = wrap_angle%(Int(i% * alpha_ratio))
      alpha = valpha%/180*PI()
      alpha_cos= alpha-(proj_angle%/180*PI())

      M = Tan(alpha)

      If M<>0 Then
         M1 = 1/M
      End If

      cell_x_pos% = camera_x% Div cell_dx%
      cell_y_pos% = camera_y% Div cell_dy%

      FirstVertIntersection valpha%, M,  cell_x_pos%, cell_y_pos%
      xv = first_v_int(0)
      yv = first_v_int(1)
      wall_type% = GetVertWallKey%( valpha%, xv, yv )      

      While wall_type% = 0
         VertIntersection xv, yv, valpha%, M
         xv = v_int(0)
         yv = v_int(1)
         wall_type% = GetVertWallKey%( valpha%, xv, yv )
      Wend

      vert_wt% = wall_type%

      xh = first_h_int(0)
      yh = first_h_int(1)
      FirstHorzIntersection valpha%, M1, cell_x_pos%, cell_y_pos%
      wall_type% = GetHorzWallKey%( valpha%, xh, yh )      

      While wall_type% = 0
         HorzIntersection xh, yh, valpha%, M1
         xh = h_int(0)
         yh = h_int(1)
         wall_type% = GetHorzWallKey%( valpha%, xh, yh )      
      Wend

      horz_wt% = wall_type%

      inters% = ComputeDistance%(alpha, valpha%, xh, yh, xv, yv)

      top% = dy%-(Int(kscale*distance*Cos(alpha_cos)))/2
      If top%<0 Then top% = dy%/2 
      If top%>dy% Then top%=dy%
   
      gamma= 1000/(distance + 0.0001)
      
      If inters% = 1 Then
         wall_type% = horz_wt%
      Else
         wall_type% = vert_wt%
      End If

      c% = color%(wall_type%) 
      r% = Int((((c% bShr 16) bAnd &hff) bShr 4) * gamma)
      g% = Int((((c% bShr 8) bAnd &hff) bShr 4) * gamma)
      b% = Int((c% bAnd &hff) bShr 4) * gamma

      If r%>255 Then r%=255
      If g%>255 Then g%=255
      If b%>255 Then b%=255

      c% = (r% bShl 16) + (g% bShl 8) + b%

      x% = (i%-ray%) * fact%
      FillRect offsetx%+xres%-x%, offsety%, offsetx%+xres%-x%+fact%, top%+offsety%, 0
      FillRect offsetx%+xres%-x%, top%+offsety%, offsetx%+xres%-x%+fact%, dy%-top%+offsety%, c%
      FillRect offsetx%+xres%-x%, dy%+offsety%, offsetx%+xres%-x%+fact%, offsety%+top%, 0
   Next i%

End Sub

