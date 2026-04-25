# VS Code Debugger Roadmap

## Phase 1: Text Programs

Goal: make the VS Code debugger reliable for text-only nuBASIC programs.

Current focus:
- dedicated console debug binary (`nubasicdebug.exe`)
- prompt/startup stability over redirected stdin/stdout
- VS Code breakpoints
- continue / step / stop
- variables and current line reporting

Success criteria:
- `F5` starts a text nuBASIC program reliably
- breakpoints stop on the expected source line
- `Continue` and `Step` keep the session responsive
- the process exits cleanly when the debug session ends or stdin closes

## Phase 2: Graphics Programs

Goal: keep the debugger as a console-controlled backend while rendering graphics in a separate window.

Planned shape:
- `nubasicdebug.exe` remains the debug backend used by VS Code
- graphic statements render into a separate helper window
- debugger commands continue to flow through stdin/stdout
- breakpoints, stepping, variables, and stack stay in the console debug session

Notes:
- the graphics window should be independent from the debugger transport
- phase 2 starts only after phase 1 is stable for text programs
- macOS: graphics stay disabled until a supported renderer exists
- Linux: graphics should follow the Windows model when possible; if `xterm`
  is not available, graphics rendering stays disabled
