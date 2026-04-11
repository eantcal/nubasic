' test_main_entry.bas
' Tests the main() entry point with argc and argv().
' ARGS: hello world

Function main(argc As Integer, argv() As String) As Integer
    Dim pass% As Integer
    Dim fail% As Integer
    pass% = 0
    fail% = 0

    ' argc should be 3: script + hello + world
    If Str$(argc) = "3" Then
        Print "  PASS  argc=3"
        pass% = pass% + 1
    Else
        Print "  FAIL  argc=" + Str$(argc) + " expected=3"
        fail% = fail% + 1
    End If

    If argv(1) = "hello" Then
        Print "  PASS  argv(1)=hello"
        pass% = pass% + 1
    Else
        Print "  FAIL  argv(1)='" + argv(1) + "' expected='hello'"
        fail% = fail% + 1
    End If

    If argv(2) = "world" Then
        Print "  PASS  argv(2)=world"
        pass% = pass% + 1
    Else
        Print "  FAIL  argv(2)='" + argv(2) + "' expected='world'"
        fail% = fail% + 1
    End If

    Print ""
    Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
    If fail% = 0 Then
        Print "ALL TESTS PASSED"
    Else
        Print "SOME TESTS FAILED"
    End If

    main = fail%
End Function
