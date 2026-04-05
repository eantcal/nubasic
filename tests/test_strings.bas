' test_strings.bas
' Tests:
'   1.  Len
'   2.  Left$
'   3.  Right$
'   4.  Mid$
'   5.  InStr  (case-insensitive, 0-based, -1 = not found)
'   6.  InStrCS (case-sensitive, 0-based, -1 = not found)
'   7.  UCase$ / LCase$
'   8.  Chr$ / Asc
'   9.  Space$
'  10.  Str$ / Val
'  11.  String concatenation (+)
'  12.  Substr$
'  13.  Hex$

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

Dim s$ As String
s$ = "Hello, World!"

' -----------------------------------------------------------------------
' 1. Len
' -----------------------------------------------------------------------
Print "--- 1. Len ---"

AssertEq "len('Hello, World!')", Str$(Len(s$)), "13"
AssertEq "len('')",              Str$(Len("")),  "0"
AssertEq "len('A')",             Str$(Len("A")), "1"

' -----------------------------------------------------------------------
' 2. Left$
' -----------------------------------------------------------------------
Print "--- 2. Left$ ---"

AssertEq "left$('Hello, World!', 5)", Left$(s$, 5), "Hello"
AssertEq "left$('Hello, World!', 0)", Left$(s$, 0), ""
AssertEq "left$('AB', 1)",            Left$("AB", 1), "A"

' -----------------------------------------------------------------------
' 3. Right$
' Right$( s$, n ) returns the last n characters.
' "Hello, World!" (13 chars): last 6 = "World!"
' -----------------------------------------------------------------------
Print "--- 3. Right$ ---"

AssertEq "right$('Hello, World!', 6)", Right$(s$, 6), "World!"
AssertEq "right$('Hello, World!', 1)", Right$(s$, 1), "!"
AssertEq "right$('AB', 1)",            Right$("AB", 1), "B"

' -----------------------------------------------------------------------
' 4. Mid$
' -----------------------------------------------------------------------
Print "--- 4. Mid$ ---"

AssertEq "mid$('Hello, World!', 8, 5)", Mid$(s$, 8, 5), "World"
AssertEq "mid$('Hello, World!', 1, 5)", Mid$(s$, 1, 5), "Hello"
AssertEq "mid$('ABCDE', 3, 2)",         Mid$("ABCDE", 3, 2), "CD"

' -----------------------------------------------------------------------
' 5. InStr — case-insensitive, 0-based index, -1 if not found
' -----------------------------------------------------------------------
Print "--- 5. InStr (case-insensitive, 0-based) ---"

' "Hello World": H=0,e=1,l=2,l=3,o=4, =5,W=6,o=7,r=8,l=9,d=10
AssertEq "instr found 'world' (0-based 6)", Str$(InStr("Hello World", "world")), "6"
AssertEq "instr found 'Hello' (0-based 0)", Str$(InStr("Hello World", "Hello")), "0"
AssertEq "instr not found -> -1",           Str$(InStr("Hello World", "xyz")),   "-1"

' -----------------------------------------------------------------------
' 6. InStrCS — case-sensitive, 0-based index, -1 if not found
' -----------------------------------------------------------------------
Print "--- 6. InStrCS (case-sensitive, 0-based) ---"

AssertEq "instrcs 'World' at 6",   Str$(InStrCS("Hello World", "World")), "6"
AssertEq "instrcs 'world' -> -1",  Str$(InStrCS("Hello World", "world")), "-1"
AssertEq "instrcs 'bB' at 2",      Str$(InStrCS("aAbBcC", "bB")),         "2"

' -----------------------------------------------------------------------
' 7. UCase$ / LCase$
' -----------------------------------------------------------------------
Print "--- 7. UCase$/LCase$ ---"

AssertEq "ucase$('hello')", UCase$("hello"), "HELLO"
AssertEq "lcase$('WORLD')", LCase$("WORLD"), "world"
AssertEq "ucase$(lcase$('NuBASIC'))", UCase$(LCase$("NuBASIC")), "NUBASIC"

' -----------------------------------------------------------------------
' 8. Chr$ / Asc
' -----------------------------------------------------------------------
Print "--- 8. Chr$/Asc ---"

AssertEq "chr$(65)", Chr$(65), "A"
AssertEq "chr$(97)", Chr$(97), "a"
AssertEq "asc('A')", Str$(Asc("A")), "65"
AssertEq "asc('a')", Str$(Asc("a")), "97"
AssertEq "chr$(asc('Z'))", Chr$(Asc("Z")), "Z"

' -----------------------------------------------------------------------
' 9. Space$
' -----------------------------------------------------------------------
Print "--- 9. Space$ ---"

AssertEq "len(space$(5))",    Str$(Len(Space$(5))), "5"
AssertEq "len(space$(0))",    Str$(Len(Space$(0))), "0"
AssertEq "'A'+space$(2)+'B'", "A" + Space$(2) + "B", "A  B"

' -----------------------------------------------------------------------
' 10. Str$ / Val
' Note: Str$(float) may produce trailing zeros (e.g. "3.140000").
' For exact float round-trip, compare numeric values with Abs().
' -----------------------------------------------------------------------
Print "--- 10. Str$/Val ---"

AssertEq "str$(42)",   Str$(42),  "42"
AssertEq "str$(-7)",   Str$(-7),  "-7"
AssertEq "val('123')", Str$(Val("123")), "123"
AssertEq "val('-99')", Str$(Val("-99")), "-99"

Dim vf As Double
vf = Val("3.14")
AssertEq "val('3.14') numeric ok", Str$(Abs(vf - 3.14) < 0.0001), "1"

' -----------------------------------------------------------------------
' 11. String concatenation
' -----------------------------------------------------------------------
Print "--- 11. Concatenation ---"

Dim a$ As String
Dim b$ As String
a$ = "foo"
b$ = "bar"
AssertEq "foo+bar",       a$ + b$,             "foobar"
AssertEq "empty + str",   "" + a$,             "foo"
AssertEq "chain concat",  a$ + "-" + b$ + "!", "foo-bar!"

' -----------------------------------------------------------------------
' 12. Substr$ — requires 3 args: Substr$(s$, 0-based-pos, length)
' -----------------------------------------------------------------------
Print "--- 12. Substr$ ---"

' "Hello World": H=0..., W=6, World = pos 6, len 5
AssertEq "substr$('Hello World',6,5)", Substr$("Hello World", 6, 5), "World"
' "ABCDE": A=0,B=1,C=2,D=3,E=4 → CDE = pos 2, len 3
AssertEq "substr$('ABCDE',2,3)",       Substr$("ABCDE", 2, 3),       "CDE"
AssertEq "substr$('ABCDE',0,2)",       Substr$("ABCDE", 0, 2),       "AB"

' -----------------------------------------------------------------------
' 13. Hex$
' -----------------------------------------------------------------------
Print "--- 13. Hex$ ---"

AssertEq "hex$(255)", Hex$(255), "ff"
AssertEq "hex$(16)",  Hex$(16),  "10"
AssertEq "hex$(0)",   Hex$(0),   "0"

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
