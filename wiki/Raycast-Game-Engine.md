# Raycast Game Engine (WinRayCast)

← [Graphics and Multimedia](Graphics-and-Multimedia) | Next: [Command Reference](Command-Reference)

---

nuBASIC 2.0 integrates **WinRayCast**, a lightweight raycasting engine that lets a
BASIC program render pseudo-3D, *Wolfenstein 3D*-style first-person scenes from a simple
2D grid map. WinRayCast provides the low-level rendering and game logic (textured walls,
sprites, doors, actors, weapons, pickups, sound, multi-level transitions); nuBASIC
provides the scripting layer, the graphics window, HUD drawing, and the game loop.

The engine is exposed to BASIC as a set of `Ray…` built-in functions (the `raycast`
module). A program reads input, advances the simulation, renders a frame, and blits it
to the GDI window — typically 30–60 times per second.

> **Platform.** WinRayCast is **Windows-only**. `RayPresent` and `RayKeyDown` rely on
> GDI and the Win32 message queue. On Linux/macOS the module is not compiled in and only
> `RayAvailable()` is defined (it returns `0`).

---

## Availability and build

WinRayCast is controlled by the `NUBASIC_WITH_RAYCAST` CMake option, which **defaults to
`ON` on Windows** and `OFF` elsewhere. The `raycast-release` / `raycast-debug` CMake
presets enable it explicitly:

```bat
cmake --preset raycast-release
cmake --build --preset raycast-release
```

When the engine is compiled in, the whole `Ray…` API is registered and `RayAvailable()`
returns `1`. When it is not, only `RayAvailable()` exists (returning `0`), so a portable
program can guard its use:

```basic
If RayAvailable() = 0 Then
   Print "This program needs a Windows build with WinRayCast."
   End
End If
```

See [Building from Source](Building-from-Source) for the full option reference.

---

## Activating the module

The `Ray…` names live in the `raycast` module. In `Syntax Modern` programs, import it
with `Using raycast`; the graphics/HUD helpers and a few math/string helpers are usually
imported alongside it:

```basic
Syntax Modern
Using raycast
Using graphics
Using math
```

In `Syntax Legacy` (the default flat namespace) the `Ray…` functions are callable
directly without a `Using` statement, exactly like any other built-in.

---

## The render loop

A WinRayCast program follows a fixed lifecycle: initialise once, load a world or project,
then run a per-frame loop of **input → update → render → present**, and shut down at the
end.

```basic
Syntax Modern
Using raycast
Using graphics

Const VIEW_W = 640
Const VIEW_H = 400

If RayAvailable() = 0 Then Print "WinRayCast not available" : End

Screen 1                                  ' WinRayCast renders into the GDI window
RaySetBaseDir(GetAppPath())               ' resolve asset paths relative to the exe
RayInit(VIEW_W, VIEW_H)                   ' create the session / framebuffer
RayLoadProject("raycast_demo/worlds/demo.world.json")

Dim running As Integer : running = 1
While running
   ScreenLock

   ' --- input ---------------------------------------------------------
   If RayKeyDown(27) Then running = 0          ' ESC = 27
   If RayKeyDown(38) Then RayMove(8)           ' Up arrow
   If RayKeyDown(40) Then RayMove(-8)          ' Down arrow
   If RayKeyDown(37) Then RayTurn(-3)          ' Left arrow
   If RayKeyDown(39) Then RayTurn(3)           ' Right arrow

   ' --- simulation ----------------------------------------------------
   RayUpdate(0.016)                            ' advance ~16 ms of world time

   ' --- render + present ---------------------------------------------
   RayRender(VIEW_W, VIEW_H)
   RayPresent(0, 0, VIEW_W, VIEW_H)            ' blit framebuffer to (x, y, w, h)

   ScreenUnlock
   MDelay 16                                   ' pace to ~60 fps
Wend

RayShutdown()
```

Key points:

- `Screen 1` must be active — WinRayCast draws into the GDI console window, so a `-t` /
  text-mode (`Screen 0`) run cannot present frames.
- `RayRender` fills an off-screen framebuffer; `RayPresent` copies it into a rectangle of
  the window. HUD/minimap drawing with the normal [graphics](Graphics-and-Multimedia)
  primitives can be layered on top before `ScreenUnlock`.
- `RayUpdate(dt)` advances doors, sprite animations, actors/enemies, pickups, effects and
  weapon bobbing. `dt` is clamped to a maximum of 0.1 s internally.

---

## Worlds and projects

WinRayCast loads scene data from JSON files rooted at a base directory (`RaySetBaseDir`):

- **`RayLoadWorld(path$)`** loads a single `*.world.json` map (walls, textures, sprites).
- **`RayLoadProject(path$)`** loads a multi-layer project — a world plus *layer
  transitions* (elevators) that move the player between levels. Use this when the demo
  spans more than one floor.

The companion demo data lives under
[`examples/raycast/raycast_demo/`](https://github.com/eantcal/nubasic/tree/main/examples/raycast/raycast_demo),
with `worlds/`, `weapons/`, sprites, textures, audio and HUD assets.

The map is a grid. Cells can be queried with `RayCellKind` (0 = empty, 1 = wall,
2 = door), `RayIsSolidCell`, and `RayKeyAtCell` (0 = none, 1 = green, 2 = red, 3 = blue) —
useful for drawing a minimap. `RayMapRows` / `RayMapCols` give the grid size and
`RayCellDx` / `RayCellDy` the cell size in world units.

---

## API reference

Coordinates returned by player queries are in **world units**; facing is in **degrees**.
Most action functions return `1` on success and `0` on failure; query functions return
the value described. All functions are Windows-only except `RayAvailable`.

### Lifecycle

| Function | Returns | Description |
|----------|---------|-------------|
| `RayAvailable()` | Integer | `1` if the engine is compiled in, else `0`. Always defined. |
| `RayInit(width, height)` | Integer | Create the session and set the framebuffer projection size (minimum 16×16). |
| `RaySetBaseDir(path$)` | Integer | Set the base directory used to resolve relative world/asset paths. |
| `RayShutdown()` | Integer | Stop background music and release the session. |

### World and project loading

| Function | Returns | Description |
|----------|---------|-------------|
| `RayLoadWorld(path$)` | Integer | Load a single `*.world.json` map. |
| `RayLoadProject(path$)` | Integer | Load a multi-layer project (world + layer transitions). |
| `RayCurrentLayer$()` | String | ID of the active layer (`RayCurrentLayer` is an alias). |

### Rendering and simulation

| Function | Returns | Description |
|----------|---------|-------------|
| `RayUpdate(dt#[, moving%])` | Integer | Advance the world by `dt` seconds (doors, animations, actors, pickups, effects). Optional `moving` flag drives weapon-bob physics. |
| `RayRender(width, height)` | Integer | Render the scene into the internal framebuffer. |
| `RayPresent(x, y, width, height)` | Integer | Blit the framebuffer into the given window rectangle (GDI). |
| `RayFrameHash()` | Integer | FNV-1a hash of the current framebuffer (deterministic regression testing). |

### Player position and camera

| Function | Returns | Description |
|----------|---------|-------------|
| `RayPlayerX()` / `RayPlayerY()` | Double | Player position in world units. |
| `RayPlayerFacing()` | Double | Facing direction in degrees (0–360). |
| `RaySetPlayer(x#, y#, facing#)` | Integer | Teleport the player to a position and facing. |
| `RayMove(distance#)` | Integer | Move forward (positive) / backward (negative); collision-aware. |
| `RayStrafe(distance#)` | Integer | Move sideways, perpendicular to facing. |
| `RayTurn(degrees#)` | Integer | Rotate the view by the given angle. |
| `RayPlayerSlope()` / `RaySetPlayerSlope(slope%)` | Integer | Get/set vertical look slope (negative = up, positive = down). |
| `RayPlayerViewCenter()` / `RaySetPlayerViewCenter(c#)` | Double / Integer | Get/set the vertical camera centre, clamped to 0.35–0.65 (0.5 = centred). |

### Map and world queries

| Function | Returns | Description |
|----------|---------|-------------|
| `RayMapRows()` / `RayMapCols()` | Integer | Grid dimensions. |
| `RayCellDx()` / `RayCellDy()` | Integer | Cell size in world units. |
| `RayIsSolidCell(row%, col%)` | Integer | `1` if the cell is solid (or out of bounds). |
| `RayCellKind(row%, col%)` | Integer | `0` empty, `1` wall, `2` door. |
| `RayKeyAtCell(row%, col%)` | Integer | Uncollected key at cell: `0` none, `1` green, `2` red, `3` blue. |
| `RayMapUnlockCount()` | Integer | Number of collected map-unlock items (computers). |

### Sprites, actors and progress

| Function | Returns | Description |
|----------|---------|-------------|
| `RaySpriteCount()` | Integer | Total sprites in the scene. |
| `RayActorCount()` | Integer | Total actors (enemies, patrollers…). |
| `RayEnemyCount()` | Integer | Hostile, countable enemies. |
| `RayKilledEnemyCount()` | Integer | Enemies killed so far. |
| `RayItemCount()` | Integer | Pickup items in the scene. |
| `RayCollectedItemCount()` | Integer | Items collected so far. |
| `RayDestroyedObjectCount()` | Integer | Destructible objects destroyed. |
| `RayPlayerStandingOn(fragment$, radiusCells#)` | Integer | `1` if the player is within `radiusCells` of a sprite whose name contains `fragment` (case-insensitive). |

### Weapons

| Function | Returns | Description |
|----------|---------|-------------|
| `RayLoadWeapon(path$)` | Integer | Load and equip a weapon from its `*.weapon.json` metadata. |
| `RayHasWeapon(path$)` | Integer | `1` if the weapon is already in the inventory. |
| `RayWeaponAmmo()` | Integer | Ammo in the current magazine. |
| `RayWeaponReserveAmmo()` | Integer | Reserve ammo for the current weapon. |
| `RayWeaponMaxAmmo()` | Integer | Maximum ammo capacity. |
| `RayReloadWeapon()` | Integer | Reload (plays the reload animation); `1` if a reload happened. |

### Combat, health and audio

| Function | Returns | Description |
|----------|---------|-------------|
| `RayDamageEnemy(damage#, rangeCells#, fovDegrees#)` | Integer | Damage the nearest target in a cone: `0` miss, `1` damaged, `2` enemy killed, `3` object destroyed. |
| `RayConsumePlayerDamage()` | Double | Damage dealt to the player since the last call (clears the accumulator). |
| `RayConsumePlayerHealing()` | Double | Healing received since the last call (clears the accumulator). |
| `RaySetPlayerEnergy(current#, max#)` | Integer | Set the player's energy/health and its maximum. |
| `RayPlaySound(path$)` | Integer | Play a sound file relative to the project directory. |

### Input

| Function | Returns | Description |
|----------|---------|-------------|
| `RayKeyDown(vkey%)` | Integer | `1` if the given Windows virtual-key code is currently down (asynchronous poll). |

### Layer transitions (elevators)

| Function | Returns | Description |
|----------|---------|-------------|
| `RayTransitionActive()` | Integer | `1` while a layer transition is in progress. |
| `RaySetTransitionManual(manual%)` | Integer | `1` = player chooses the destination, `0` = automatic. |
| `RayTransitionOptionCount()` | Integer | Number of available transition destinations. |
| `RayTransitionOptionLayer$(index%)` | String | Destination layer ID of option `index` (1-based; `RayTransitionOptionLayer` is an alias). |
| `RayTransitionSelected()` | Integer | Currently selected option (1-based; `0` if none). |
| `RaySelectTransition(index%)` | Integer | Select an option (wraps around); returns the new 1-based selection. |
| `RayConfirmTransition()` | Integer | Trigger the selected transition; `1` if confirmed. |

---

## Full example

[`examples/raycast/eclipse_protocol.bas`](https://github.com/eantcal/nubasic/blob/main/examples/raycast/eclipse_protocol.bas)
is a complete, heavily-commented playable demo built on WinRayCast. It is written as a
tutorial — a `GAME SETTINGS` block at the top collects the tunable parameters, and the
main loop shows the full input → update → render → present cycle in context. It exercises
most of the API above: multi-weapon combat with ammo and reloads, energy/health and
damage feedback, a minimap and HUD, key/lock doors, difficulty scaling, autosave
checkpoints, and elevator transitions between levels. It also doubles as a showcase of
nuBASIC 2.0 language features, combining `Syntax Modern`, `Struct`, and `Class` with the
classic graphics primitives.

Run it from a Windows build that includes WinRayCast (see [IDE](IDE) or the
[README](https://github.com/eantcal/nubasic#readme) for launch options).

---

← [Graphics and Multimedia](Graphics-and-Multimedia) | Next: [Command Reference](Command-Reference)
