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

## What the engine does, and what you write

WinRayCast is a thin **engine** with a BASIC **driver**. The split is worth understanding,
because it explains why the API looks the way it does: a handful of commands to *drive* the
engine, and a set of queries to *read back* what happened so your BASIC code can render a
HUD and enforce game rules.

**The engine does the heavy lifting** — all of this happens inside `RayUpdate`,
`RayRender`, and `RayPresent`, not in your code:

- **Wall rendering** — casts one ray per screen column, computes wall distances, and
  texture-maps walls, floor, ceiling, and the sky/horizon into an off-screen framebuffer
  (`RayRender`).
- **Sprites** — projects, depth-sorts, and animates every sprite in the scene.
- **Doors** — opens/closes them based on player proximity and key ownership, and plays the
  door sounds (`RayUpdate`).
- **Actors / enemies** — the actor system moves enemies, makes them chase and attack, and
  accumulates the damage they deal to the player (`RayUpdate`).
- **Pickups** — automatically collects items in range (health, ammo, keys, weapons) and
  accumulates healing.
- **View weapon** — animates the weapon, applies walk-bob, and tracks magazine/reserve ammo.
- **Layer transitions** — animates elevators and swaps the active level.
- **Audio** — starts/loops the world's background music and plays sound effects.
- **Collision** — `RayMove` / `RayStrafe` are collision-aware; the engine refuses moves
  into solid cells.
- **Presentation** — copies the framebuffer into the GDI window (`RayPresent`).

**Your BASIC program is the game** — the engine has no game loop, no input policy, and no
HUD of its own. You write:

- the **window and loop** — `Screen 1`, and a `While` loop paced with `MDelay`;
- **lifecycle** — `RayInit`, `RaySetBaseDir`, `RayLoadWorld` / `RayLoadProject`, and
  `RayShutdown`;
- **input → intent** — read keys with `RayKeyDown` and translate them into `RayMove`,
  `RayStrafe`, `RayTurn`, look (`RaySetPlayerSlope`), fire (`RayDamageEnemy`), reload
  (`RayReloadWeapon`), and weapon switching (`RayLoadWeapon`);
- the **per-frame `RayUpdate` → `RayRender` → `RayPresent` sequence**;
- **health bookkeeping** — drain the engine's accumulators with `RayConsumePlayerDamage` /
  `RayConsumePlayerHealing`, maintain your own energy value, push it back with
  `RaySetPlayerEnergy`, and decide death/respawn;
- the **HUD and minimap** — drawn with the ordinary [graphics](Graphics-and-Multimedia)
  primitives, fed by query functions (`RayPlayerX/Y`, `RayMapRows/Cols`, `RayCellKind`,
  `RayKeyAtCell`, `RayEnemyCount`, `RayWeaponAmmo`, …);
- the **rules** — win/lose conditions, checkpoints, and the elevator-selection UI built on
  the `RayTransition…` functions.

If you never call `RayUpdate`, the world freezes (no doors, no enemies, no animation) even
though `RayRender` still draws it; if you never call `RayPresent`, nothing reaches the
screen even though the frame was rendered.

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

WinRayCast loads its scene data from JSON files, resolved relative to the base directory set
with `RaySetBaseDir`. The companion demo data lives under
[`examples/raycast/raycast_demo/`](https://github.com/eantcal/nubasic/tree/main/examples/raycast/raycast_demo),
with `worlds/`, `weapons/`, sprites, textures, audio, and HUD assets.

### The world model

A `*.world.json` file is a JSON object with these top-level keys (the demo's
`demo.world.json` is a large, fully-featured example):

| Key | Required | Meaning |
|-----|----------|---------|
| `format` | yes | Must be the string `"winraycast.world"`. |
| `version` | yes | Must be `2`. |
| `grid` | yes | `{ columns, rows, cellWidth, cellDepth, defaultWallHeight }` — map size and the size of one cell in **world units** (the demo uses 512). |
| `playerStart` | recommended | `{ xCell, yCell, facingDegrees }` — spawn position in (fractional) cell coordinates and initial heading. |
| `textures` | yes | A **texture palette**: a map of two-hex-digit ids → `{ "file": "textures/…png" }`. |
| `blocks` | yes | A **block palette**: a map of two-hex-digit ids → a cell definition (its `walls`, and optional `floor` / `ceiling` / `door`). |
| `layers` | yes | A non-empty array of levels. Each layer has an `id` and a `cells` grid. |

The map itself is **not** a grid of raw numbers — it is a grid of *block references*. Each
entry in a layer's `cells` array is a two-hex-digit string naming a block from the `blocks`
palette, and each block describes what that cell looks like (a stack of textured `walls`
between `bottom` and `top` heights, with `collision`/`passable` flags, plus an optional
floor, ceiling, or `door`). This palette approach lets a 16×16 map reuse a handful of richly
described block types.

A complete **minimal world** — a 5×5 room you can walk around in — looks like this:

```json
{
  "format": "winraycast.world",
  "version": 2,
  "grid": { "columns": 5, "rows": 5, "cellWidth": 512, "cellDepth": 512, "defaultWallHeight": 512 },
  "playerStart": { "xCell": 2.5, "yCell": 2.5, "facingDegrees": 90 },
  "textures": {
    "01": { "file": "textures/wall.png" },
    "08": { "file": "textures/floor.png" }
  },
  "blocks": {
    "00": { "name": "floor", "floor": { "texture": "08", "height": 0 }, "walls": [] },
    "01": { "name": "wall",  "walls": [ { "kind": "solid", "texture": "01", "bottom": 0, "top": 512, "collision": true } ] }
  },
  "layers": [
    {
      "id": "level_1",
      "cells": [
        ["01","01","01","01","01"],
        ["01","00","00","00","01"],
        ["01","00","00","00","01"],
        ["01","00","00","00","01"],
        ["01","01","01","01","01"]
      ]
    }
  ]
}
```

Block `01` (a full-height solid wall) lines the perimeter; block `00` (open floor) fills the
interior; the player spawns in the middle of the room. The two referenced `.png` files must
exist relative to the base directory set with `RaySetBaseDir`. Sprites, enemies, weapons,
doors with keys, and elevators are layered on top of this same model with additional
fields — see the demo data for fully-worked examples.

`RayLoadWorld(path$)` loads one such file. `RayLoadProject(path$)` loads the same file but
also reads its **layer transitions** (elevators that move the player between layers); use it
when a world has more than one level. `RayCurrentLayer$()` reports the active layer id, and
the `RayTransition…` functions drive the elevator UI.

Once a world is loaded, query the grid from BASIC with `RayMapRows` / `RayMapCols`,
`RayCellDx` / `RayCellDy` (cell size in world units), `RayCellKind` (0 = empty, 1 = wall,
2 = door), `RayIsSolidCell`, and `RayKeyAtCell` (0 = none, 1 = green, 2 = red, 3 = blue) —
everything you need to draw a minimap.

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

## Tutorial: from an empty window to a playable level

This tutorial builds a small game step by step. Steps 1–3 use the
[minimal world](#the-world-model) above (rename it `room.world.json` and place it next to
your `.bas` file, with the two `.png` textures it references); steps 4–6 switch to the
bundled demo project, which already ships enemies, weapons, and assets.

Run each step from a Windows build that includes WinRayCast (`RayAvailable()` returns `1`).

### Step 1 — Show the world

The smallest useful program: open the GDI window, initialise the engine, load the world,
and render a single frame per loop iteration. There is no movement yet, but you should see
a textured room.

```basic
Syntax Modern
Using raycast
Using graphics

Const VIEW_W = 640
Const VIEW_H = 400

If RayAvailable() = 0 Then Print "WinRayCast not available" : End

Screen 1
RaySetBaseDir(GetAppPath())          ' textures/worlds resolve from here
RayInit(VIEW_W, VIEW_H)
If RayLoadWorld("room.world.json") = 0 Then Print "Cannot load world" : End

Dim running As Integer : running = 1
While running
   ScreenLock
   If RayKeyDown(27) Then running = 0          ' ESC quits
   RayUpdate(0.016)
   RayRender(VIEW_W, VIEW_H)
   RayPresent(0, 0, VIEW_W, VIEW_H)
   ScreenUnlock
   MDelay 16
Wend
RayShutdown()
```

### Step 2 — Walk around

Add input. Map the arrow keys to forward/back movement and turning; `RayMove` and
`RayTurn` are collision-aware, so you cannot walk through walls. (Windows virtual-key codes:
arrows are 37–40, `W`/`A`/`S`/`D` are 87/65/83/68.)

```basic
' …inside the loop, before RayUpdate:
Dim moving As Integer : moving = 0
If RayKeyDown(38) Then RayMove(10)  : moving = 1     ' Up
If RayKeyDown(40) Then RayMove(-10) : moving = 1     ' Down
If RayKeyDown(37) Then RayTurn(-3)                   ' Left  (turn)
If RayKeyDown(39) Then RayTurn(3)                    ' Right (turn)
If RayKeyDown(65) Then RayStrafe(-8) : moving = 1    ' A strafe left
If RayKeyDown(68) Then RayStrafe(8)  : moving = 1    ' D strafe right

RayUpdate(0.016, moving)   ' pass the moving flag so the weapon bobs while walking
```

### Step 3 — Draw a minimap

The engine renders the 3-D view; the HUD is yours. Query the grid and the player position,
then paint a small top-down map with ordinary graphics primitives after `RayPresent` (so it
sits on top of the 3-D view):

```basic
Sub DrawMinimap(ox As Integer, oy As Integer, tile As Integer)
   Dim rows As Integer : rows = RayMapRows()
   Dim cols As Integer : cols = RayMapCols()
   Dim r As Integer, c As Integer
   For r = 0 To rows - 1
      For c = 0 To cols - 1
         Dim col As Integer
         If RayCellKind(r, c) = 1 Then col = Rgb(120,120,120) Else col = Rgb(20,20,20)
         FillRect ox + c*tile, oy + r*tile, ox + c*tile + tile-1, oy + r*tile + tile-1, col
      Next c
   Next r
   ' player dot — convert world units to cells via RayCellDx/RayCellDy
   Dim px As Integer : px = ox + Int(RayPlayerX() / RayCellDx() * tile)
   Dim py As Integer : py = oy + Int(RayPlayerY() / RayCellDy() * tile)
   FillEllipse px-2, py-2, px+2, py+2, Rgb(255,80,80)
End Sub

' …in the loop, between RayPresent and ScreenUnlock:
DrawMinimap 8, 8, 10
```

### Step 4 — Enemies and shooting

Enemies, weapons, and pickups come from scene data, so switch to the bundled demo project
(`RayLoadProject` also wires up its elevators). Equip a weapon, then fire with
`RayDamageEnemy`, which damages the nearest target inside a cone (range in cells, field of
view in degrees) and returns what it hit:

```basic
RayLoadProject("raycast_demo/worlds/demo.world.json")
RayLoadWeapon("weapons/pistol/pistol.weapon.json")

' …in the loop:
If RayKeyDown(17) Then                       ' Ctrl = fire
   Dim hit As Integer
   hit = RayDamageEnemy(25.0, 6.0, 12.0)     ' 25 dmg, 6-cell range, 12° cone
   If hit = 2 Then Print "Enemy down!"
End If
If RayKeyDown(82) Then RayReloadWeapon()      ' R = reload
```

Progress queries let you detect victory: `RayEnemyCount()` is the total, `RayKilledEnemyCount()`
the tally so far.

### Step 5 — Health and a HUD

The engine accumulates the damage enemies deal and the healing from pickups; **you** own the
player's energy. Each frame, drain the accumulators, update your value, push it back with
`RaySetPlayerEnergy`, and draw a bar and an ammo readout:

```basic
Dim energy As Double : energy = 100.0

' …in the loop, after RayUpdate:
energy = energy - RayConsumePlayerDamage() + RayConsumePlayerHealing()
If energy < 0.0 Then energy = 0.0
RaySetPlayerEnergy(energy, 100.0)
If energy <= 0.0 Then running = 0             ' death

' …after RayPresent (HUD):
FillRect 8, VIEW_H-24, 8 + Int(energy*2), VIEW_H-12, Rgb(220,40,40)   ' energy bar
TextOut 8, VIEW_H-40, "AMMO " + Str$(RayWeaponAmmo()) + "/" + Str$(RayWeaponMaxAmmo()), Rgb(255,255,255)
```

### Step 6 — Move between levels

When the player steps onto an elevator the project defines a **transition**. Let them pick a
destination and confirm it:

```basic
RaySetTransitionManual(1)        ' player chooses (0 = automatic)

' …in the loop:
If RayTransitionOptionCount() > 0 Then
   If RayKeyDown(37) Then RaySelectTransition(RayTransitionSelected() - 1)
   If RayKeyDown(39) Then RaySelectTransition(RayTransitionSelected() + 1)
   If RayKeyDown(13) Then RayConfirmTransition()    ' Enter
End If
If RayTransitionActive() Then TextOut 8, 8, "Travelling to " + RayCurrentLayer$(), Rgb(255,255,0)
```

From here, the full [`eclipse_protocol.bas`](https://github.com/eantcal/nubasic/blob/main/examples/raycast/eclipse_protocol.bas)
demo adds difficulty scaling, autosave checkpoints, a richer HUD, key/lock doors, and a
death/respawn overlay — all on top of exactly these building blocks.

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
