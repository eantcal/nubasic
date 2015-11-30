#!/usr/local/bin/nubasic
Rem Simple game
Rem This program challenges the user to guess a randomly selected number 
Rem within the 1-10 range, without offering the usual hints of "higher"/"lower":
Rem See also http://en.wikipedia.org/wiki/QBasic

Cls
Print "Guess the number!"

' An input statement, that takes what the user inputs...
Input "Would you like to play? (Y/N): ", choice$     

' and decides whether or not you want to play:
If Left$(LCase$(choice$),1) = "y" Then                


  ' Set up number of guess remaining
  guesses% = 5
                
  ' Sets up the random number generator
  Randomize                                   

  ' Picks a random number between 1 and 10 (inclusive)
  target% = Int(Rnd(1) * 10) + 1  

  ' Sets up a flag called 'won%' to check if user has won
  won% = 0                                           
  Print "The number is between 1 and 10."

  Rem Enters a loop until the user wins or runs out of chances
  While guesses% > 0 And won% = 0                    
    Input "Enter your guess: ", guess%  : Rem Takes user input (the guess)

    ' Determines if the guess was correct
    If guess% = target% Then
      Print "Correct, the answer was "; target%; "!"
      won% = 1  :' Sets a flag to indicate user has won
    Else
      guesses% = guesses% - 1  :' Deducts one chance
      Print "Sorry, please try again. You have "; guesses%; " guesses left."
    End If

  Wend : Rem End of guessing loop

  If won% = 0 Then Print "You ran out of guesses, the number was "; target%; "."
End If

