' test_line_continuation.bas - underscore joins physical source lines
' EXPECT_OUTPUT: joined expression ok|declare continuation ok|comment continuation ok

Syntax Modern

Using runtime

Declare Function lstrlenA Lib "kernel32.dll" _
    Alias "lstrlenA" _
    (text As String) As Integer

Dim text$ As String
text$ = "joined " _
    + "expression ok"

Print text$

If lstrlenA("hello") = 5 Then
    Print "declare continuation ok"
End If

Print "comment " _ ' trailing comments after the continuation marker are ignored
    + "continuation ok"
