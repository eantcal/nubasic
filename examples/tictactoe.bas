#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  TicTacToe for nuBASIC (tictactoe.bas)
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

Dim Board$(3)

player_cnt% = 0
computer_cnt% = 0
cell_size% = 100
winSymb$ = " "
playerSymbol$ = "o"
computerSymbol$ = "x"
computerStarts% = 1

expertMode% = 0
computerScore% = 0
playerScore% = 0

' Call MainLoop
MainLoop

' -----------------------------------------------------------------------------

Sub DrawBackgroundImage()
   PlotImage "bg.bmp", 0,0
End Sub


' -----------------------------------------------------------------------------

Sub DrawBoard()
   UpdateBoard 110, 90, cell_size%
End Sub


' -----------------------------------------------------------------------------

Sub NewGame()

   Randomize

   Board$(0) = "   "
   Board$(1) = "   "
   Board$(2) = "   "

   player_cnt% = 0
   computer_cnt% = 0
   winSymb$ = " "

   computerStarts% = computerStarts% bXor 1

   If computerStarts% Then
      If playerSymbol$ = "o" Then
         playerSymbol$ = "x"
         computerSymbol$ = "o"
      Else
         If playerSymbol$ = "x" Then
            playerSymbol$ = "o"
            computerSymbol$ = "x"
         End If
      End If
   End If

End Sub


' -----------------------------------------------------------------------------

Sub DrawCell(x%, y%, cellsize%, color%)
   FillRect x%, y%, x%+cellsize%, y%+cellsize%, 0
   Rect x%, y%, x%+cellsize%, y%+cellsize%, color%
End Sub


' -----------------------------------------------------------------------------

Sub ProcessPlayerAction(x%, y%, cs%, psym$, xm%, ym%)
  
   
   For i%=0 to 2
      For j%=0 to 2
         xx% = x%+j%*cs%
         yy% = y%+i%*cs%

         If xm%>xx% And xm%<=xx%+cs% And ym%>yy% And ym%<=yy%+cs% Then

            s$ = SubStr$(Board$(i%), j%, 1)

            If  s$= " " Then
               Board$(i%) = pStr$(Board$(i%), j%, psym$)
               player_cnt% = player_cnt% + 1
               Exit Sub
            End If

         End If

      Next j%
   Next i%

End Sub


' -----------------------------------------------------------------------------

Sub DrawXO(x%, y%, cellsize%, color%, xoid%)

   d2% = cellsize% Div 4

   If xoid% = 0 Then
      Ellipse x%+d2%, y%+d2%, x%+d2%*3, y%+d2%*3, color%
      Ellipse x%+d2%-1, y%+d2%-1, x%+d2%*3+1, y%+d2%*3+1, color%
   Else
      Line x%+d2%, y%+d2%, x%+d2%*3, y%+d2%*3, color%
      Line x%+d2%*3, y%+d2%, x%+d2%, y%+d2%*3, color%

      Line x%+d2%+1, y%+d2%, x%+d2%*3+1, y%+d2%*3, color%
      Line x%+d2%*3+1, y%+d2%, x%+d2%+1, y%+d2%*3, color%
   End If

End Sub


' -----------------------------------------------------------------------------

Sub UpdateBoard(x%, y%, cs%)

   If expertMode%=1 Then e$ = "Expert" Else e$="Beginner"

   FillRect x%, y% - 60, x% + cs%*3, y%, &hFFF
   TextOut x%+20, y% - 50, "TicTacToe 1.3 (" + e$ + ")", &h66ff11

   score$ = "nuBASIC " + str$(computerScore%) + " - You "+ str$(playerScore%)
   TextOut x%+20, y% - 30,  score$, &hFFF0E
   
   For i%=0 to 2
      For j%=0 to 2
         xx% = x%+j%*cs%
         yy% = y%+i%*cs%

         DrawCell xx%, yy%, cs%, &hffffff      

         s$ = SubStr$(Board$(i%), j%, 1)

         If  s$= "x" Then
               DrawXO xx%, yy%, cs%, &hA050A0, 1
         Else
            If  s$= "o" Then
               DrawXO xx%, yy%, cs%, &hA0A000, 0
            End If
         End If

      Next j%
   Next i%

   If winSymb$ <> " " Then Exit Sub

   If 3 = ChkDiag1%("o") Then
      winSymb$ = "o"
   Else
      If 3 = ChkDiag1%("x") Then
         winSymb$ = "x"
      End If
   End If

   If winSymb$<>" " Then
      Line x%, y%, x%+3*cs%, y%+3*cs%, &hff0000
      Exit Sub
   End If

   
   If 3 = ChkDiag2%("o") Then
      winSymb$ = "o"
   Else
      If 3 = ChkDiag2%("x") Then
         winSymb$ = "x"
      End If
   End If

   If winSymb$<>" " Then
      Line x%+3*cs%, y%, x%, y%+3*cs%, &hff0000
      Exit Sub
   End If

   For i% = 0 to 2
      If 3 = CheckForRow%("o", i%) Then
         winSymb$ = "o"
      Else
         If 3 = CheckForRow%("x", i%) Then
            winSymb$ = "x"
         End If
      End If

      If winSymb$<>" " Then
         yy% = y%+cs%*i%+(cs% Div 2)
         
         Line x%, yy%, x%+3*cs%, yy%, &hff0000
         Exit Sub
      End If
   Next i%

   For i% = 0 to 2
      If 3 = CheckForCol%("o", i%) Then
         winSymb$ = "o"
      Else
         If 3 = CheckForCol%("x", i%) Then
            winSymb$ = "x"
         End If
      End If

      If winSymb$<>" " Then
         xx% = x%+cs%*i%+(cs% Div 2)
         
         Line xx%, y%, xx%, y%+3*cs%, &hff0000
         Exit Sub
      End If
   Next i%


End Sub


' -----------------------------------------------------------------------------

Function ChkDiag1%(symb$)

   ChkDiag1% = 0

   For i%=0 to 2
      If symb$ = SubStr$(Board$(i%), i%, 1) Then 
         ChkDiag1% = ChkDiag1% + 1
      End If
   Next i%

End Function


' -----------------------------------------------------------------------------

Function ChkCompDiag1%()
   ChkCompDiag1% = ChkDiag1%(playerSymbol$) + ChkDiag1%(computerSymbol$) 
End Function


' -----------------------------------------------------------------------------

Function ChkDiag2%(symb$)

   ChkDiag2% = 0

   For i%=2 to 0 Step -1
      If symb$ = SubStr$(Board$(2-i%), i%, 1) Then 
         ChkDiag2% = ChkDiag2% + 1
      End If
   Next i%

End Function



' -----------------------------------------------------------------------------

Function ChkCompDiag%()
   ChkCompDiag% = ChkDiag2%(playerSymbol$) + ChkDiag2%(computerSymbol$) 
End Function



' -----------------------------------------------------------------------------

Function CheckForRow%(symb$, r%)
   CheckForRow% = 0

   For i% = 0 to 2
      If symb$ = SubStr$(Board$(r%), i%, 1) Then
         CheckForRow% = CheckForRow% + 1
      End If
   Next i%

End Function


' -----------------------------------------------------------------------------

Function CheckForCol%(symb$, c%)

   CheckForCol% = 0

   For i%=0 to 2
      If symb$ = SubStr$(Board$(i%), c%, 1) Then
         CheckForCol% = CheckForCol% + 1      
      End If
   Next i%

End Function


' -----------------------------------------------------------------------------

Sub CloseDiag1(symb$)
   For i%=0 to 2
      If " " = SubStr$(Board$(i%), i%, 1) Then 
         Board$(i%) = pStr$(Board$(i%), i%, symb$)
         computer_cnt% = computer_cnt% + 1
         Exit Sub
      End If
   Next i%
End Sub


' -----------------------------------------------------------------------------

Sub CloseDiag2(symb$)
   For i%=0 to 2
      If " " = SubStr$(Board$(i%), 2-i%, 1) Then 
         Board$(i%) = pStr$(Board$(i%), 2-i%, symb$)
         computer_cnt% = computer_cnt% + 1
         Exit Sub
      End If
   Next i%
End Sub


' -----------------------------------------------------------------------------

Sub CloseRow(r%, symb$)

   For i% = 0 to 2
      If " " = SubStr$(Board$(r%), i%, 1) Then
         Board$(r%) = pStr$(Board$(r%), i%, symb$)
         computer_cnt% = computer_cnt% + 1
         Exit Sub
      End If
   Next i%

End Sub


' -----------------------------------------------------------------------------

Sub CloseCol(c%, symb$)

   For i% = 0 to 2
      If " " = SubStr$(Board$(i%), c%, 1) Then
         Board$(i%) = pStr$(Board$(i%), c%, symb$)
         computer_cnt% = computer_cnt% + 1
         Exit Sub
      End If
   Next i%

End Sub


' -----------------------------------------------------------------------------

Sub OpenGame(symb$)
   ' First turn
   If computer_cnt% = 0 Then
      If SubStr$(Board$(1), 1, 1) = " " Then
         Board$(1) = pStr$(Board$(1), 1, symb$)
         computer_cnt% = computer_cnt% + 1
         Exit Sub
      End If

      If SubStr$(Board$(0), 0, 1) = " " Then
         Board$(0) = pStr$(Board$(0), 0, symb$)
         computer_cnt% = computer_cnt% + 1
         Exit Sub
      End If

      If SubStr$(Board$(2), 2, 1) = " " Then
         Board$(2) = pStr$(Board$(2), 2, symb$)
         computer_cnt% = computer_cnt% + 1
         Exit Sub
      End If
   End If

End Sub


' -----------------------------------------------------------------------------

Sub ProcessComputerAction(symb$)

   
   If symb$="x" Then playerSymb$="o" Else playerSymb$="x"

   ' ----------------------------------------------------------------------
   ' If computer has to play as begginer just use random free cell And put 
   ' symbol inside it
   If expertMode%=0 Then
      
      found% = 0

      While Not(isTheEnd%())

         i% = Int(Rnd(1) * 100) Mod 3
         j% = Int(Rnd(1) * 100) Mod 3
         
         If (SubStr$(Board$(i%), j%, 1)=" ") Then
            Board$(i%) = pStr$(Board$(i%), j%, symb$)
            Exit Sub
         End If
      Wend
      
      
      Exit Sub
   End If

   ' ----------------------------------------------------------------------
   ' ... Else play to win !!!

   If computer_cnt% = 0 Then
      OpenGame symb$
      Exit Sub
   End If


   ' Verify If we can close or player may close on diagonals

   me% = ChkDiag1%(symb$)
   player% = ChkDiag1%(playerSymb$)

   If (me%=2 And player%=0) or (me%=0 And player%=2) Then
      CloseDiag1 symb$
      Exit Sub
   End If

   me% = ChkDiag2%(symb$)
   player% = ChkDiag2%(playerSymb$)

   If (me%=2 And player%=0) or (me%=0 And player%=2) Then
      CloseDiag2 symb$
      Exit Sub
   End If

   
   ' Verify If we can close on rows

   For row% = 0 to 2

      me% = CheckForRow%(symb$, row%)
      player% = CheckForRow%(playerSymb$, row%)

      If (me%=2 And player%=0) Then
         CloseRow row%, symb$
         Exit Sub
      End If

   Next row%


   ' Verify If we can close on columns

   For col% = 0 to 2

      me% = CheckForCol%(symb$, col%)
      player% = CheckForCol%(playerSymb$, col%)

      If (me%=2 And player%=0) Then
         CloseCol col%, symb$
         Exit Sub
      End If

   Next col%


   ' Verify If player may close on rows

   For row% = 0 to 2

      me% = CheckForRow%(symb$, row%)
      player% = CheckForRow%(playerSymb$, row%)

      If (me%=0 And player%=2) Then
         CloseRow row%, symb$
         Exit Sub
      End If

   Next row%


   ' Verify If player may close on columns

   For col% = 0 to 2

      me% = CheckForCol%(symb$, col%)
      player% = CheckForCol%(playerSymb$, col%)

      If (me%=0 And player%=2) Then
         CloseCol col%, symb$
         Exit Sub
      End If

   Next col%


   ' Nobody can close, compute optimal solution 

   me% = ChkDiag1%(symb$)
   player% = ChkDiag1%(playerSymb$)

   If (me%=2 And player%=0) Then
      CloseDiag1 symb$
      Exit Sub
   End If

   me% = ChkDiag2%(symb$)
   player% = ChkDiag2%(playerSymb$)

   If (me%=2 And player%=0) Then
      CloseDiag2 symb$
      Exit Sub
   End If

   
   If ChkCompDiag1%()=3 And computer_cnt%=1 Then
      If (SubStr$(Board$(1), 1, 1)=symb$) Then
         Board$(2) = pStr$(Board$(2), 1, symb$) 
      Else
         Board$(2) = pStr$(Board$(2), 0, symb$)
      End If
      computer_cnt% = computer_cnt% + 1
      Exit Sub
   End If
   
   If ChkCompDiag%()=3 And computer_cnt%=1 Then
      Board$(0) = pStr$(Board$(0), 1, symb$)
      computer_cnt% = computer_cnt% + 1
      Exit Sub
   End If

   
   For cnt%=0 to 1

   
      For row% = 0 to 2

         me% = CheckForRow%(symb$, row%)
         player% = CheckForRow%(playerSymb$, row%)

         If (me%=(2-cnt%) And player%=cnt%) Then
            CloseRow row%, symb$
            Exit Sub
         End If

      Next row%


      For col% = 0 to 2

         me% = CheckForCol%(symb$, col%)
         player% = CheckForCol%(playerSymb$, col%)

         If (me%=(2-cnt%) And player%=cnt%) Then
            CloseCol col%, symb$
            Exit Sub
         End If

      Next col%
   
   Next cnt%


   ' Ok try to fill an empty cell
   For row% = 2 to 0 Step -1
      me% = CheckForRow%(symb$, row%)
      player% = CheckForRow%(playerSymb$, row%)

      If (me%+player%)<3 Then
         CloseRow row%, symb$
         Exit For
      End If
   Next row%


End Sub


' -----------------------------------------------------------------------------

Function isTheEnd%()
   isTheEnd% = 1

   If winSymb$<>" " Then 
      Exit Function
   End If

   For i%=0 to 2 
      For j% = 0 to 2
         If SubStr$(Board$(i%), j%, 1)=" " Then   
            isTheEnd% = 0
            Exit Function
         End If
      Next j%
   Next i%
   
End Function


' -----------------------------------------------------------------------------

Sub GetDifficultSettings()

   DrawBackgroundImage

   TextOut 150,  80, "Welcome to TicTacToe ", Rgb(255,255,255)
   TextOut 150, 120, "Click to choose ", Rgb(255,255,255)
   
   FillRect 140, 160, 300, 200, Rgb(255,255,0)
   TextOut 150, 170,  "Beginner", Rgb(255,0,0)

   FillRect 140, 220, 300, 260, Rgb(255,255,0)
   TextOut 150, 230, "Expert", Rgb(255,0,0)
   
   btn% = GetMouseBtn()
   expertMode% = 2

   While expertMode% > 1
      btn% = 0

      While btn% = 0 
         btn% = GetMouseBtn()
         x% = GetMouseX()
         y% = GetMouseY()
      Wend

      If x%>=140 And x%<=300 And y%>=160 And y%<=200 Then
         Rect 140, 160, 300, 200, Rgb(0,255,0)
         expertMode% = 0
      End If

      If x%>=140 And x%<=300 And y%>=220 And y%<=260 Then
         Rect 140, 220, 300, 260, Rgb(0,255,0)
         expertMode% = 1  
      End If
   Wend

   
End Sub


Sub MainLoop()
   ' -----------------------------------------------------------------------------
   ' Main Loop
   ' -----------------------------------------------------------------------------

   MoveWindow GetWindowX(),GetWindowY(), 600,480 
   FillRect 0,0,600,480,0

   GetDifficultSettings
   Delay 1

   While 1
      NewGame

      DrawBoard

      
      If playerSymbol$ = "x" Then computerSymbol$ = "o" Else computerSymbol$ = "x"

      If computerStarts% Then
         ProcessComputerAction computerSymbol$
         DrawBoard
      End If

      While 1
         
         btn% = GetMouseBtn()
         xm% = GetMouseX()
         ym% = GetMouseY()
         
         If btn% Then
            
            oldcnt% = player_cnt%
            
            ProcessPlayerAction cell_size%, cell_size%, cell_size%, playerSymbol$, xm%, ym%
            DrawBoard
         

            If oldcnt% <> player_cnt% Then

               If Not(isTheEnd%()) Then
                  ProcessComputerAction computerSymbol$
                  MDelay 100
                  DrawBoard
               End If 
            
               If isTheEnd%() Then

                  xmsg% = cell_size%*2
                  ymsg% = cell_size%*2+cell_size% Div 2

                  FillRect xmsg%-10, ymsg%-10, xmsg%+150, ymsg%+60, Rgb(128,128,255)

                  If winSymb$<>" " Then
                     If winSymb$ = playerSymbol$ Then
                        TextOut xmsg%, ymsg%, "   You won !", Rgb(0,255,0)
                        playerScore% = playerScore% + 1
                     Else
                        TextOut xmsg%, ymsg%, "  You lose !", Rgb(255,0,0)
                        computerScore% = computerScore% + 1
                     End If
                  Else
                     TextOut xmsg%, ymsg%, "  Nobody won !", Rgb(255,255,0)
                  End If

                  TextOut xmsg%, ymsg%+30, "Click to continue", Rgb(255,255,255)
         
                  Exit While

               End If
            End If
         End If
      Wend

      While GetMouseBtn() = 0 
      Wend

      Delay 1

      DrawBackgroundImage

   Wend

   End

End Sub
