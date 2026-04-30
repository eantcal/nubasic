' SKIP: covered by RunDebuggerContract
Print "start"
x = Foo()
Print "after"
End

Function Foo()
Print "foo1"
Foo = 42
End Function
