' test_raycast.bas
' PLATFORM: windows
' EXPECT_OUTPUT: available=1|loaded=1|x=2304|y=5376|facing=90|x2=2303|y2=5408|hash=1900823333

Using raycast

Print "available="; RayAvailable()

RayInit 320, 200

loaded% = RayLoadWorld("examples/raycast/raycast_demo/worlds/demo.world.json")
Print "loaded="; loaded%

If loaded% = 0 Then
    End
End If

Print "x="; RayPlayerX()
Print "y="; RayPlayerY()
Print "facing="; RayPlayerFacing()

RayMove 32
RayTurn 15
RayRender 320, 200

Print "x2="; RayPlayerX()
Print "y2="; RayPlayerY()
Print "hash="; RayFrameHash()
