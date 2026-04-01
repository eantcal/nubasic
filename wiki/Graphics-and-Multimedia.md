# Graphics and Multimedia

← [Language Reference](Language-Reference) | Next: [Command Reference](Command-Reference)

---

All graphics functions are available in the full build (Windows GDI or Linux/X11). They are
absent in the *tiny* build, which targets minimal environments.

The coordinate system has its origin at the **top-left** corner of the working window client
area, with X increasing to the right and Y increasing downward. All coordinates are in pixels.

## Colors

Colors are packed RGB integers in GDI byte order (BGR stored in the low three bytes), so the
layout is `&hBBGGRR`:

| Color   | nuBASIC constant |
|---------|-----------------|
| Red     | `&h0000FF`      |
| Green   | `&h00FF00`      |
| Blue    | `&hFF0000`      |
| Yellow  | `&h00FFFF`      |
| Cyan    | `&hFFFF00`      |
| Magenta | `&hFF00FF`      |
| White   | `&hFFFFFF`      |
| Black   | `&h000000`      |

The `Rgb(r, g, b)` function computes a color from separate 0–255 components:

```basic
skyBlue%   = Rgb(135, 206, 235)
sunYellow% = Rgb(255, 220, 50)
```

---

## Drawing Primitives

### Lines and Shapes

```basic
Line x1, y1, x2, y2, color%          ' straight line
Rect x1, y1, x2, y2, color%          ' rectangle outline
FillRect x1, y1, x2, y2, color%      ' filled rectangle
Ellipse x1, y1, x2, y2, color%       ' ellipse outline (bounding box)
FillEllipse x1, y1, x2, y2, color%   ' filled ellipse
```

### Pixels

```basic
SetPixel x%, y%, color%       ' write a pixel
c% = GetPixel(x%, y%)         ' read a pixel's color
```

### Text on Screen

`TextOut` draws a text string at a pixel coordinate. Unlike `Print`, it does not move the
text cursor or scroll the buffer.

```basic
TextOut x%, y%, text$, color%

TextOut 10,  10, "Score: " + Str$(score%), &hFFFFFF
TextOut 200, 240, "GAME OVER", Rgb(255, 50, 50)
```

### Bitmap Images

`PlotImage` loads a BMP file and draws it at a pixel coordinate (native size, no scaling):

```basic
PlotImage "background.bmp", 0, 0
PlotImage "sprite.bmp", hero_x%, hero_y%
```

### Complete Scene Example

```basic
Cls
MoveWindow GetWindowX(), GetWindowY(), 640, 480
FillRect 0, 0, 640, 480, Rgb(30, 30, 60)          ' dark night sky
FillEllipse 520, 20, 600, 100, Rgb(255, 255, 200)  ' moon
FillRect 0, 360, 640, 480, Rgb(0, 80, 0)           ' green ground
For i% = 0 To 5
   cx% = 80 + i% * 100
   FillEllipse cx%, 200, cx%+60, 360, Rgb(20, 120, 20)  ' trees
   FillRect cx%+25, 330, cx%+35, 365, Rgb(80, 50, 20)   ' trunks
Next i%
TextOut 10, 10, "nuBASIC Graphics Demo", &hFFFFFF
```

---

## Flicker-free Rendering

### The Problem

When multiple drawing commands execute in sequence, each one immediately blits to the screen.
The user sees each intermediate state — a cleared frame, a partial board, and so on — as
visible flicker.

### The Solution: Double Buffering

| Instruction | Effect |
|-------------|--------|
| `ScreenLock` | Suspend automatic screen refresh; all drawing goes to the back buffer |
| `ScreenUnlock` | Present the back buffer to the screen in one blit; resume automatic refresh |
| `Refresh` | Force an immediate blit; lock state is unchanged |

### Pattern 1 — Game Frame Loop

```basic
While Not(game_over%)
   ScreenLock
   FillRect 0, 0, 640, 480, &h000000   ' clear previous frame
   DrawBoard
   DrawPlayer player_x%, player_y%
   DrawEnemies
   DrawHUD score%, lives%
   ScreenUnlock
   MDelay 16                            ' pace to ~60 fps
Wend
```

### Pattern 2 — Progressive Rendering (Mandelbrot)

```basic
For x0 = -2 To 2 Step 0.013
   ScreenLock
   For y0 = -1.5 To 1.5 Step 0.013
      ' ... compute c ...
      FillRect x0*d%+dx%, y0*d%+dy%, x0*d%+dx%+2, y0*d%+dy%+2, c%*16
   Next y0
   ScreenUnlock   ' one column appears per iteration
Next x0
```

### Pattern 3 — Atomic Erase + Redraw

```basic
x_old% = x%  :  y_old% = y%
x% = x% + dx%  :  y% = y% + dy%

ScreenLock
FillEllipse x_old%*10, y_old%*10, x_old%*10+10, y_old%*10+10, 0        ' erase old
FillEllipse x%*10,     y%*10,     x%*10+10,     y%*10+10,     &hffffff ' draw new
ScreenUnlock
```

### Pattern 4 — Dialog Before Blocking Input

```basic
ScreenLock
FillRect 150, 220, 490, 310, &hffff00
Rect     150, 220, 490, 310, &h000000
TextOut  180, 245, "Game over! Play again? (Y/N)", &h000000
ScreenUnlock
Refresh             ' ensure dialog is visible before blocking
key$ = Input$(1)
```

---

## Mouse Input

Mouse input is polled — there is no event queue.

```basic
btn% = GetMouseBtn()   ' 0=none, 1=left, 2=middle, 4=right
x%   = GetMouseX()     ' cursor X in pixels from left edge
y%   = GetMouseY()     ' cursor Y in pixels from top edge
```

### Hit-testing a Button Region

```basic
bx1% = 40  :  by1% = 60
bx2% = 200 :  by2% = 100

FillRect bx1%, by1%, bx2%, by2%, &hffff00
TextOut bx1%+20, by1%+15, "Click me", &h000000

While 1
   btn% = GetMouseBtn()
   mx%  = GetMouseX()
   my%  = GetMouseY()

   If btn% = 1 And mx% >= bx1% And mx% <= bx2% And my% >= by1% And my% <= by2% Then
      Print "Button clicked!"
      MDelay 200
   End If
   MDelay 16
Wend
```

### Drawing with the Mouse

```basic
Cls
FillRect 0, 0, 640, 480, &h000000

While 1
   key$ = InKey$()
   If key$ = "q" Or key$ = "Q" Then Exit While
   If GetMouseBtn() = 1 Then
      SetPixel GetMouseX(), GetMouseY(), &hffffff
   End If
   MDelay 5
Wend
```

---

## Sound and Window Management

### Window Control

```basic
MoveWindow GetWindowX(), GetWindowY(), 800, 600

Print "Position: "; GetWindowX(); ", "; GetWindowY()
Print "Size:     "; GetWindowDx(); " x "; GetWindowDy()
Print "Canvas:   "; GetSWidth(); " x "; GetSHeight()

SetTopMost()   ' keep window above all others
```

Typical startup sequence:

```basic
Cls
MoveWindow 100, 100, 640, 480
FillRect 0, 0, 640, 480, &h000000
```

### Sound

```basic
PlaySound "background.wav", 1   ' async: returns immediately
PlaySound "explosion.wav", 0    ' sync: waits for completion
Beep                            ' system beep
```

### Message Boxes

```basic
result% = MsgBox("nuBASIC Demo", "Setup complete. Ready to play?")
If result% > 0 Then
   Print "User confirmed."
End If
```

---

← [Language Reference](Language-Reference) | Next: [Command Reference](Command-Reference)
