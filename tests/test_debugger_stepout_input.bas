' SKIP: covered by RunDebuggerContract
Print "start"
Call Worker()
Print "after"
End

Sub Worker()
Print "before input"
Input$ 1
Print "after input"
End Sub
