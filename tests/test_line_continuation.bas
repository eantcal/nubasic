' test_line_continuation.bas - underscore joins physical source lines
' EXPECT_OUTPUT: joined expression ok|declare continuation ok|comment continuation ok

Syntax Modern

Using runtime

' Test 1: expression continuation
Dim text$ As String
text$ = "joined " _
    + "expression ok"

Print text$

' Test 2: declare continuation — multi-line Sub declaration uses _
Sub CheckContinuation(prefix$ As String, _
                      suffix$ As String)
    Print prefix$ + suffix$
End Sub

Call CheckContinuation("declare continuation", _
                       " ok")

' Test 3: comment after continuation marker is ignored
Print "comment " _ ' trailing comments after the continuation marker are ignored
    + "continuation ok"
