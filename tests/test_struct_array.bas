' test_struct_array.bas
' Tests:
'   1. Array of structs: Dim arr(n) As Point
'   2. Read/write individual array elements by literal index
'   3. Read/write array elements by loop variable
'   4. Struct with an array field: field(n) As Type
'   5. GetDateTime() returns a struct with correct fields

Screen 0
Dim pass% As Integer
Dim fail% As Integer
pass% = 0
fail% = 0

Sub AssertEq(label$, got$, expected$)
    If got$ = expected$ Then
        Print "  PASS  " + label$
        pass% = pass% + 1
    Else
        Print "  FAIL  " + label$ + "  got='" + got$ + "'  expected='" + expected$ + "'"
        fail% = fail% + 1
    End If
End Sub

Sub AssertTrue(label$, cond%)
    If cond% Then
        Print "  PASS  " + label$
        pass% = pass% + 1
    Else
        Print "  FAIL  " + label$
        fail% = fail% + 1
    End If
End Sub

' -----------------------------------------------------------------------
' 1. Array of structs - literal index
' -----------------------------------------------------------------------
Print "--- 1. Array of structs (literal index) ---"

Struct Point
    x As Double
    y As Double
End Struct

Dim pts(3) As Point
pts(0).x = 1.0 : pts(0).y = 10.0
pts(1).x = 2.0 : pts(1).y = 20.0
pts(2).x = 3.0 : pts(2).y = 30.0

AssertEq "pts(0).x", Str$(pts(0).x), "1"
AssertEq "pts(0).y", Str$(pts(0).y), "10"
AssertEq "pts(1).x", Str$(pts(1).x), "2"
AssertEq "pts(1).y", Str$(pts(1).y), "20"
AssertEq "pts(2).x", Str$(pts(2).x), "3"
AssertEq "pts(2).y", Str$(pts(2).y), "30"

' -----------------------------------------------------------------------
' 2. Loop variable index
' -----------------------------------------------------------------------
Print "--- 2. Array of structs (loop index) ---"

Dim i% As Integer
For i% = 0 To 2
    pts(i%).x = (i% + 1) * 5.0
    pts(i%).y = (i% + 1) * 50.0
Next i%

AssertEq "pts(0).x via loop", Str$(pts(0).x), "5"
AssertEq "pts(1).x via loop", Str$(pts(1).x), "10"
AssertEq "pts(2).x via loop", Str$(pts(2).x), "15"
AssertEq "pts(0).y via loop", Str$(pts(0).y), "50"
AssertEq "pts(1).y via loop", Str$(pts(1).y), "100"
AssertEq "pts(2).y via loop", Str$(pts(2).y), "150"

' -----------------------------------------------------------------------
' 3. Struct with an array field
' -----------------------------------------------------------------------
Print "--- 3. Struct with array field ---"

Struct Vec3
    v(3) As Double
End Struct

Dim vec As Vec3
vec.v(0) = 1.5
vec.v(1) = 2.5
vec.v(2) = 3.5

AssertEq "vec.v(0)", Str$(vec.v(0)), "1.5"
AssertEq "vec.v(1)", Str$(vec.v(1)), "2.5"
AssertEq "vec.v(2)", Str$(vec.v(2)), "3.5"

' -----------------------------------------------------------------------
' 4. GetDateTime() returns a struct consistent with scalar functions
' -----------------------------------------------------------------------
Print "--- 4. GetDateTime() struct ---"

Dim dt As DateTime
dt = GetDateTime()

AssertEq "dt.year  = SysYear",  Str$(dt.year),   Str$(SysYear())
AssertEq "dt.month = SysMonth", Str$(dt.month),  Str$(SysMonth())
AssertEq "dt.day   = SysDay",   Str$(dt.day),    Str$(SysDay())
AssertEq "dt.hour  = SysHour",  Str$(dt.hour),   Str$(SysHour())
AssertEq "dt.minute= SysMin",   Str$(dt.minute), Str$(SysMin())
AssertEq "dt.second= SysSec",   Str$(dt.second), Str$(SysSec())

AssertTrue "dt.year  >= 2025", dt.year  >= 2025
AssertTrue "dt.month in 1-12", dt.month >= 1 And dt.month <= 12
AssertTrue "dt.day   in 1-31", dt.day   >= 1 And dt.day   <= 31
AssertTrue "dt.hour  in 0-23", dt.hour  >= 0 And dt.hour  <= 23
AssertTrue "dt.minute in 0-59",dt.minute >= 0 And dt.minute <= 59
AssertTrue "dt.second in 0-59",dt.second >= 0 And dt.second <= 59

' -----------------------------------------------------------------------
' Summary
' -----------------------------------------------------------------------
Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
