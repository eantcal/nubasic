#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  ray.bas sample is part of nuBASIC - (c) 2014 - acaldmail@gmail.com
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
Dim map2D$(64)
Dim color%(16)
version$="1.0"

cell_dx% = 20
cell_dy% = 20

player_x% = cell_dx% * 7 + 8
player_y% = cell_dy% * 7 + 8

visual_alpha% = 60
xres% = 320
ratio% = 2
yres% = 300
alpha_ratio = visual_alpha% / xres%
distance = 0
wall_type% = 0
kscale = 0.9

Dim first_h_int(2)
Dim first_v_int(2)

Dim h_int(2)
Dim v_int(2)


' -----------------------------------------------------------------------------
' Program entry point
Main
' -----------------------------------------------------------------------------


' -----------------------------------------------------------------------------
' Procedures
' -----------------------------------------------------------------------------

' -----------------------------------------------------------------------------
Sub Main()
' -----------------------------------------------------------------------------
   SetupGame
   MainLoop
End Sub


' -----------------------------------------------------------------------------
Sub MainLoop()
' -----------------------------------------------------------------------------
   a%=0
   offset% = 0
   first_time% = 1
   

   While 1
      key$ = InKey$()
      If Len(key$) or first_time% Then
         first_time% = 0

         If key$ = "m" Then 
            a%=a%+15
         ElseIf key$ = "n" Then 
            a%=a%-15
         ElseIf key$ = "a" Then
            offset% = 10
         ElseIf key$ = "z" Then
            offset% = -10
         End If

         a% = wrap_angle%(a%)
         ap% = wrap_angle%(a% + 30)
         
         a = (ap%/180)*PI()

         If offset% <> 0 Then            
            x% =  player_x% - Cos(a) * offset%
            y% =  player_y% - Sin(a) * offset%
            
            k% = GetWallKey%(ap%, x%, y%)

            If k%=0 Then
               player_x% = x%
               player_y% = y%
            End If
            
            offset% = 0            
         End If

         Draw3DView 0, 0, yres%, Int(a%/(alpha_ratio)), ratio%
      End If
   Wend
End Sub


' -----------------------------------------------------------------------------
Sub SetupGame()
' -----------------------------------------------------------------------------
   MoveWindow GetWindowX(),GetWindowY(), 800, 480
   FillRect 0,0,800,500,0
   SetupColors
   SetupMap
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
   For y% = 0 to 15
      For x% = 0 to 15
         c$ = Mid$(map2D$(y%), x%+1, 1) 
        
         c% = Val(c$)
      
         xx% = x% * scale% + ox%
         yy% = y% * scale% + oy%

         FillRect xx%, yy%, xx% + scale%, yy% + scale%, color%(c%)

      Next x%
   Next y%

   px% = Int(scale% * player_x% / cell_dx%)
   py% = Int(scale% * player_y% / cell_dy%)

   FillEllipse ox% + px%-3, oy% + py%-3, ox% + px%+3, oy% + py%+3, &hff

End Sub


' -----------------------------------------------------------------------------
Sub FirstHorzIntersection(valpha%, M1, cell_x_pos%, cell_y_pos%) 
' -----------------------------------------------------------------------------
  ' yi
  If Not(valpha% >= 180 And valpha% < 360) Then 
     first_h_int(1) = cell_y_pos% * cell_dy%
  Else   
     first_h_int(1) = (cell_y_pos%+1) * cell_dy%   
  End If

  yp = player_y%
  xp = player_x%
  first_h_int(0) = M1 * (first_h_int(1) - yp) + xp

End Sub



' -----------------------------------------------------------------------------
Sub FirstVertIntersection(valpha%, M, cell_x_pos%, cell_y_pos%) 
' -----------------------------------------------------------------------------
  ' xi
   If valpha% >= 90 And valpha% < 270 Then 
      first_v_int(0) = (cell_x_pos%+1) * cell_dx%             
   Else
      first_v_int(0) = (cell_x_pos%) * cell_dx%             
   End If

  ' yi
   yp = player_y%
   xp = player_x%
   first_v_int(1) = M * (first_v_int(0) - xp) + yp   

End Sub


' -----------------------------------------------------------------------------
Sub VertIntersection(x,y, valpha%, M)
' -----------------------------------------------------------------------------
   If Not(valpha% >= 90 And valpha% < 270) Then
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
   If (valpha% >= 0 And valpha% < 180) Then
      h_int(0) = x - M1 * cell_dx% : REM xi
      h_int(1) = y - cell_dy%      : REM yi
   Else 
      h_int(0) = x + M1 * cell_dx% : REM xi
      h_int(1) = y + cell_dy%      : REM yi
   End If
End Sub


' -----------------------------------------------------------------------------
function GetWallKey%( a%, x%, y% )
' -----------------------------------------------------------------------------
       
   c% = Int(x% Div cell_dx%)
   r% = Int(y% Div cell_dy%)

   If c%>= 16 Then 
      c%=15
   Else
      If c%<0 Then c% = 0
   End If 

   If r%>= 16 Then
      r%=15
   Else
     If r%<0 Then r%=0
   End If

   row$ = map2D$(r%)

   GetWallKey% = Val%( Mid$( row$, c%+1, 1 ) )
End function


' -----------------------------------------------------------------------------
function GetVertWallKey%( valpha%, x, y )
' -----------------------------------------------------------------------------
   c% = Int(x / cell_dx%)
   r% = Int(y / cell_dy%)
   
   If valpha% > 270 or valpha% < 90 Then c% = c% - 1


   If c%>= 16 Then 
      c%=15
   Else
      If c%<0 Then c% = 0
   End If 

   If r%>= 16 Then
      r%=15
   Else
     If r%<0 Then r%=0
   End If

   row$ = map2D$(r%)

   GetVertWallKey% = Val%( Mid$( row$, c%+1, 1 ) )
End function


' -----------------------------------------------------------------------------
function GetHorzWallKey%( valpha%, x, y )
' -----------------------------------------------------------------------------
   c% = Int(x / cell_dx%)
   r% = Int(y / cell_dy%)

   If (valpha% >= 0 And valpha%< 180) Then r% = r% - 1

   If c%>= 16 Then 
      c%=15
   Else
      If c%<0 Then c%= 0
   End If 

   If r%>= 16 Then
      r%=15
   Else
      If r%<0 Then r%=0
   End If

   row$ = map2D$(r%)

   GetHorzWallKey% = Val%( Mid$( row$, c%+1, 1 ) )
End function

   
' -----------------------------------------------------------------------------
function ComputeDistance%(alpha, valpha%, xh, yh, xv, yv)
' -----------------------------------------------------------------------------
   dv = Abs((player_x%-xv) / (Cos(alpha)+0.000001 )) 
   dh = Abs((player_y%-yh) / (Sin(alpha)+0.000001 ))

   If dh<dv Then 
      distance = dh 
      ComputeDistance% = 1
   Else 
      distance = dv
      ComputeDistance% = 2
   End If
   
End function


' -----------------------------------------------------------------------------
function wrap_angle%(a%)
' -----------------------------------------------------------------------------
   If a%<0 Then
      wrap_angle% = 360 + a% 
   ElseIf a%>=360 Then 
      wrap_angle%=a%-360
   Else 
      wrap_angle% = a%      
   End If


End function


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

      cell_x_pos% = player_x% Div cell_dx%
      cell_y_pos% = player_y% Div cell_dy%

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

      inters% = ComputeDistance%(alpha, valpha%, xh, yh, xv, yv)

      top% = dy%-(Int(kscale*distance*cos(alpha_cos)))/2
      If top%<0 Then top% = dy%/2 
      If top%>dy% Then top%=dy%
   
      gamma%= Int(10000/(distance + 0.0001))
      If gamma%>200 Then gamma%=200
      
      c% = (Int(gamma%+20) bShl 16) + (Int(gamma%+30) bShl 8) + gamma% 

      x% = (i%-ray%) * fact%
      FillRect offsetx%+x%, offsety%, offsetx%+x%+fact%, top%+offsety%, &h402020
      FillRect offsetx%+x%, top%+offsety%, offsetx%+x%+fact%, dy%-top%+offsety%, c%
      FillRect offsetx%+x%, dy%+offsety%, offsetx%+x%+fact%, offsety%+top%, &h206020   
   Next i%

   DrawMap2D offsetx%, offsety%+dy%
   Textout offsetx% + 100, offsety%+dy%+20, "nuBASIC - Ray " + version$, &hffffff
   Textout offsetx% + 100, offsety%+dy%+50, "MOVE    /\\  'a'     \\/  'z'", &hffffff
   Textout offsetx% + 100, offsety%+dy%+80, "ROTATE    ACW 'n'   CW 'm'", &hffffff
End Sub


' -----------------------------------------------------------------------------
Sub SetupMap()
' -----------------------------------------------------------------------------
   map2D$(0) = "1111111111111111"
   map2D$(1) = "1000000000000001" 
   map2D$(2) = "1000000000001001" 
   map2D$(3) = "1000110000111001" 
   map2D$(4) = "1000110000000001" 
   map2D$(5) = "1000000000000001" 
   map2D$(6) = "1000000000000001" 
   map2D$(7) = "1000111000000001" 
   map2D$(8) = "1000111000011001" 
   map2D$(9) = "1000100000011001" 
   map2D$(10)= "1000100000000001" 
   map2D$(11)= "1000111000000001" 
   map2D$(12)= "1000111000000111" 
   map2D$(13)= "1000000000000111" 
   map2D$(14)= "1000000000001111" 
   map2D$(15)= "1111111111111111" 
End Sub
