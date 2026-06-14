' test_system_millis.bas
' EXPECT_OUTPUT: millis ok

Using system

startMs = Millis()
MDelay(5)
endMs = Millis()

If endMs >= startMs Then
    Print "millis ok"
Else
    Print "millis failed"
End If
