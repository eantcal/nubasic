#!/usr/local/bin/nubasic
' -----------------------------------------------------------------------------
'  This program is a 1D-convolution example for nuBASIC (conv.bas)
' -----------------------------------------------------------------------------

' Define vectors v1 and v2
Dim v1(5) As Double
Dim v2(3) As Double

v1(0) = 0.2
v1(1) = 0.5
v1(2) = -0.4
v1(3) = 1.0
v1(4) = 0.0

v2(0) = 0.6
v2(1) = 1.9
v2(2) = -2.2

' Create a result vector (1 is a place holder)
' the actual vector size depends on result of conv function
Dim result(1) As Double

' Compute the 1D-convolution of v1 and v2
result = conv(v1, v2, sizeof(v1), sizeof(v2))
'is equivalent to conv(v1, v2), because size of vectors is optional 

Cls

' Print out the content of vector result
For i = 0 To sizeof(result) - 1
  print "v(";i;")=";result(i)
Next i

