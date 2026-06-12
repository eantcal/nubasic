Using raycast

Print "Raycast available: "; RayAvailable()

If RayAvailable() = 0 Then
    Print "Raycast module is not built in."
    End
End If

RayInit 320, 200

world$ = "examples/raycast/raycast_demo/worlds/demo.world.json"
If RayLoadWorld(world$) = 0 Then
    Print "Cannot load world: "; world$
    End
End If

Print "Player X: "; RayPlayerX()
Print "Player Y: "; RayPlayerY()
Print "Facing : "; RayPlayerFacing()

RayMove 32
RayTurn 15
RayRender 320, 200

Print "After move X: "; RayPlayerX()
Print "After move Y: "; RayPlayerY()
Print "Frame hash  : "; RayFrameHash()
