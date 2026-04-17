' test_multisource.bas
' Tests splitting one application across multiple source files with Include.

Syntax Modern
Using String

Include "multisource/domain.bas"
#Include "multisource/services.bas"

Dim pass% As Integer
Dim fail% As Integer
pass% = 0
fail% = 0

Sub AssertEq(label$ As String, got$ As String, expected$ As String)
    If got$ = expected$ Then
        Print "  PASS  " + label$
        pass% = pass% + 1
    Else
        Print "  FAIL  " + label$ + "  got='" + got$ + "'  expected='" + expected$ + "'"
        fail% = fail% + 1
    End If
End Sub

Print "--- 1. Source Include / #Include ---"

Dim counter As MultiCounter
counter = New MultiCounter("core", 2)
counter.Add(DefaultStep%())
Call AddTwice(counter, 4)

AssertEq "class from included source", counter.Report$(), "core=13"
AssertEq "nested include function", Str$(ScaleValue%(5)), "15"
AssertEq "nested include string", SourceTag$(), "multi-source"
AssertEq "#Include service function", ServiceSummary$(counter), "service:core=13"

Print "--- 2. Object references across source boundary ---"

Dim alias As MultiCounter
alias = counter
alias.Add(1)
AssertEq "alias mutates included class instance", counter.Report$(), "core=14"

Print ""
Print "Results: " + Str$(pass%) + " passed,  " + Str$(fail%) + " failed"
If fail% = 0 Then
    Print "ALL TESTS PASSED"
Else
    Print "SOME TESTS FAILED"
End If
