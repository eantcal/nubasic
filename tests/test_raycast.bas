' test_raycast.bas
' PLATFORM: windows
' EXPECT_OUTPUT: available=1|loaded=1|project=1|layer=level_1|transition=0|rows=16|cols=16|celldx=512|celldy=512|solid=1|door=2|map=0|key=1|pistol=1|weapon=0|slope=24|center=0.56|standing=1|sprites=32|actors=4|enemies=12|killed=0|items=39|collected=0|destroyed=0|shot=0|objectshot=3|destroyed2=1|collected2=1|healing=35|damage=36|killshot=2|killed2=1|layer2=level_0|killed3=1|collected3=4|destroyed3=1|x=2304|y=5376|facing=90|x2=2303|y2=5408|hash=2095660438|map2=1|weapon2=1|movingUpdate=1

Using raycast

Print "available="; RayAvailable()

RayInit 320, 200

loaded% = RayLoadWorld("examples/raycast/raycast_demo/worlds/demo.world.json")
Print "loaded="; loaded%

If loaded% = 0 Then
End
End If

project% = RayLoadProject("examples/raycast/raycast_demo/worlds/demo.world.json")
Print "project="; project%
Print "layer="; RayCurrentLayer$()
Print "transition="; RayTransitionActive()
Print "rows="; RayMapRows()
Print "cols="; RayMapCols()
Print "celldx="; RayCellDx()
Print "celldy="; RayCellDy()
Print "solid="; RayIsSolidCell(0, 0)
Print "door="; RayCellKind(4, 7)
Print "map="; RayMapUnlockCount()
Print "key="; RayKeyAtCell(10, 5)
Print "pistol="; RayHasWeapon("weapons/pistol/pistol.weapon.json")
Print "weapon="; RayLoadWeapon("weapons/super_shotgun/super_shotgun.weapon.json")
RaySetPlayerSlope(24)
Print "slope="; RayPlayerSlope()
RaySetPlayerViewCenter(0.56)
Print "center="; RayPlayerViewCenter()
RaySetPlayer(2304, 5888, 0)
Print "standing="; RayPlayerStandingOn("supply_crate", 0.45)
RaySetPlayerSlope(0)
RaySetPlayerViewCenter(0.5)
Print "sprites="; RaySpriteCount()
Print "actors="; RayActorCount()
Print "enemies="; RayEnemyCount()
Print "killed="; RayKilledEnemyCount()
Print "items="; RayItemCount()
Print "collected="; RayCollectedItemCount()
Print "destroyed="; RayDestroyedObjectCount()
Print "shot="; RayDamageEnemy(1.0, 0.1, 1.0)
RaySetPlayer 6400, 6912, 90
Print "objectshot="; RayDamageEnemy(50.0, 2.0, 40.0)
Print "destroyed2="; RayDestroyedObjectCount()
RaySetPlayer 2816, 5376, 0
RayUpdate 0.016
Print "collected2="; RayCollectedItemCount()
RaySetPlayer 5888, 5888, 0
RaySetPlayerEnergy(65, 100)
RayUpdate 0.016
Print "healing="; RayConsumePlayerHealing()
RaySetPlayer 2304, 5376, 90

Print "x="; RayPlayerX()
Print "y="; RayPlayerY()
Print "facing="; RayPlayerFacing()

RayMove 32
RayTurn 15
RayRender 320, 200

Print "x2="; RayPlayerX()
Print "y2="; RayPlayerY()
Print "hash="; RayFrameHash()
RaySetPlayer 7424, 7424, 0
RayUpdate 0.016
Print "map2="; RayMapUnlockCount()
RaySetPlayer 3328, 6912, 0
RayUpdate 0.016
Print "weapon2="; RayLoadWeapon("weapons/super_shotgun/super_shotgun.weapon.json")
Print "movingUpdate="; RayUpdate(0.016, 1)

RaySetPlayer 6912, 6400, 0
For i% = 1 To 80
    RayUpdate 0.016
Next i%
Print "damage="; RayConsumePlayerDamage()

RaySetPlayer 2816, 4096, 270
Print "killshot="; RayDamageEnemy(200.0, 8.0, 40.0)
Print "killed2="; RayKilledEnemyCount()

RaySetPlayer 1280, 7424, 0
For i% = 1 To 100
    RayUpdate 0.016
Next i%
Print "layer2="; RayCurrentLayer$()
Print "killed3="; RayKilledEnemyCount()
Print "collected3="; RayCollectedItemCount()
Print "destroyed3="; RayDestroyedObjectCount()
