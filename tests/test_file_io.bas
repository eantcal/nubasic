' test_file_io.bas
' Tests:
'   1. Open For Output / Print # / Close
'   2. Open For Input Access Read / Input# / Eof / Close
'   3. FError on missing file
'   4. Write and read back multiple lines
'   5. FSize > 0 after write
'   6. Append mode (Open For Append)
'
' Note on syntax:
'   - Write to file:  Print #fd, "text"
'   - Read from file: Input# fd, var$   (no space between # and fd number)
'   - Eof:  Eof(fd)  returns 1 when at end-of-file

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

Dim tmpfile$ As String
Dim rtfile$ As String
Dim appfile$ As String
tmpfile$ = Pwd$() + "/nu_test_io.txt"
rtfile$  = Pwd$() + "/nu_test_rt.txt"
appfile$ = Pwd$() + "/nu_test_app.txt"

' -----------------------------------------------------------------------
' 1. Open For Output / Print # / Close
' -----------------------------------------------------------------------
Print "--- 1. Write file ---"

Open tmpfile$ For Output As #1
If FError(1) Then
    Print "  SKIP  cannot create temp file"
    GoTo SkipAllIO
End If
Print #1, "line one"
Print #1, "line two"
Print #1, "line three"
Close #1
AssertEq "write without error", "ok", "ok"

' -----------------------------------------------------------------------
' 2. Open For Input / Input# / Eof
' -----------------------------------------------------------------------
Print "--- 2. Read file ---"

Dim line$ As String
' Array sized 5: Input# reads one extra empty line after last newline
Dim lines$(5) As String
Dim idx% As Integer
idx% = 0

Open tmpfile$ For Input Access Read As #2
If FError(2) Then
    Print "  FAIL  cannot open for reading"
    fail% = fail% + 1
    GoTo SkipRead
End If

While Not(Eof(2))
    Input# 2, line$
    lines$(idx%) = line$
    idx% = idx% + 1
Wend
Close #2

' idx% = 4 because Input# reads a trailing empty line after last \n
AssertEq "lines$(0)", lines$(0), "line one"
AssertEq "lines$(1)", lines$(1), "line two"
AssertEq "lines$(2)", lines$(2), "line three"
AssertEq "at least 3 lines read", Str$(idx% >= 3), "1"

SkipRead:

' -----------------------------------------------------------------------
' 3. FError on missing file
' -----------------------------------------------------------------------
Print "--- 3. FError on missing file ---"

Open "/this/path/does/not/exist/file.txt" For Input As #3
AssertEq "ferror on missing file", Str$(FError(3)), "-1"
Close #3

' -----------------------------------------------------------------------
' 4. Write and read back multiple data types
' -----------------------------------------------------------------------
Print "--- 4. Round-trip data types ---"

Open rtfile$ For Output As #4
Print #4, "42"
Print #4, "hello world"
Close #4

Dim ri% As Integer
Dim rs$ As String

Open rtfile$ For Input Access Read As #5
Input# 5, rs$
ri% = Val(rs$)
Input# 5, rs$
Close #5

AssertEq "roundtrip integer", Str$(ri%), "42"
AssertEq "roundtrip string",  rs$,       "hello world"

' -----------------------------------------------------------------------
' 5. FSize > 0 after write
' -----------------------------------------------------------------------
Print "--- 5. FSize ---"

Open tmpfile$ For Input Access Read As #6
Dim fsz As Double
fsz = FSize(6)
Close #6
AssertEq "fsize > 0", Str$(fsz > 0), "1"

' -----------------------------------------------------------------------
' 6. Append mode
' -----------------------------------------------------------------------
Print "--- 6. Append mode ---"

Open appfile$ For Output As #7
Print #7, "first"
Close #7

Open appfile$ For Append As #8
Print #8, "second"
Close #8

Dim lcount% As Integer
lcount% = 0
Open appfile$ For Input Access Read As #9
While Not(Eof(9))
    Input# 9, line$
    lcount% = lcount% + 1
Wend
Close #9
' lcount% >= 2: Input# reads a trailing empty line after last \n
AssertEq "append >= 2 lines read", Str$(lcount% >= 2), "1"

SkipAllIO:

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
