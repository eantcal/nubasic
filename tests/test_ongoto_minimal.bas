Dim v% As Integer
Dim r$ As String

v% = 0 : On v% Goto lbl0, lbl1, lbl2
lbl0: r$ = "zero" : Goto done
lbl1: r$ = "one"  : Goto done
lbl2: r$ = "two"
done:
If r$ = "zero" Then Print "PASS v%=0" Else Print "FAIL v%=0 got=" + r$

v% = 1 : On v% Goto lbl0b, lbl1b, lbl2b
lbl0b: r$ = "zero" : Goto done2
lbl1b: r$ = "one"  : Goto done2
lbl2b: r$ = "two"
done2:
If r$ = "one" Then Print "PASS v%=1" Else Print "FAIL v%=1 got=" + r$

v% = 2 : On v% Goto lbl0c, lbl1c, lbl2c
lbl0c: r$ = "zero" : Goto done3
lbl1c: r$ = "one"  : Goto done3
lbl2c: r$ = "two"
done3:
If r$ = "two" Then Print "PASS v%=2" Else Print "FAIL v%=2 got=" + r$
