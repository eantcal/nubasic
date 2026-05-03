# Integrated Development Environment (IDE)

← [Command Reference](Command-Reference) | Next: [History and Evolution](History-and-Evolution)

---

The nuBASIC IDE bundles an advanced syntax highlighting editor, an interpreter, and a debugger
into a single application available for Windows and Linux (GTK+2). It is the recommended tool
for writing non-trivial programs.

### What's new

The Windows IDE has gained three significant capabilities on top of the
historical editor + Build/Run/Step workflow:

- **Richer debugger** — full Step Into / Step Over / Step Out / Run to
  Cursor / Pause-Break commands; class destructors and calls inside
  expressions are stepped correctly. See [Integrated Debugger](#integrated-debugger).
- **Multi-source projects** — open a `.nbp` project file and the IDE
  loads the entry script and lets the debugger step into every
  `Include`-d file as a single program. See
  [Multi-source projects](#multi-source-projects).
- **VS Code extension** — first-class VS Code support with the same
  step modes, watch, call stack and breakpoints, backed by the
  `nubasicdebug` console-subsystem binary. See
  [VS Code extension](#vs-code-extension).

---

## Syntax Highlighting Editor

The editor is built on the [Scintilla](https://www.scintilla.org/) library.

**Main features:**

- **Syntax highlighting** — nuBASIC keywords, literals, comments, and identifiers rendered in distinct colours
- **Code folding** — sections can be collapsed to a single summary line and later expanded
- **Auto-completion** — `F12` opens a pop-up list of matching keywords; `Tab` inserts the selected item
- **Bookmarks** — any line can be bookmarked with `Ctrl+F7`; navigate with `Ctrl+F8` / `Ctrl+F9`
- **Find and Replace** — full-text search with case sensitivity, whole-word matching, and regex replacement
- **Context-sensitive help** — select a keyword and press `F1` to display the built-in help entry
- **Online help** — `Ctrl+F1` opens the relevant online help topic in a browser
- **Integrated debugger** — breakpoints, Step Into / Over / Out, Run to Cursor, Pause/Break, expression evaluation (data-tips)
- **Line and column indicator** — the toolbar permanently shows the cursor position
- **Zoom** — `F3` zooms in; `Ctrl+F3` zooms out

---

## Keyboard Commands

Editor movement keys (`Arrow`, `Page Up/Down`, `Home`, `End`) extend a stream selection when
`Shift` is held, and a rectangular selection when both `Shift` and `Alt` are held.

| Action | Key |
|--------|-----|
| Indent block | `Tab` |
| Dedent block | `Shift+Tab` |
| Delete to start of word | `Ctrl+Backspace` |
| Delete to end of word | `Ctrl+Delete` |
| Delete to start of line | `Ctrl+Shift+Backspace` |
| Delete to end of line | `Ctrl+Shift+Delete` |
| Go to start of document | `Ctrl+Home` |
| Extend selection to start of document | `Ctrl+Shift+Home` |
| Go to start of display line | `Alt+Home` |
| Go to end of document | `Ctrl+End` |
| Extend selection to end of document | `Ctrl+Shift+End` |
| Go to end of display line | `Alt+End` |
| Scroll up | `Ctrl+Up` |
| Scroll down | `Ctrl+Down` |
| Line cut | `Ctrl+L` |
| Line delete | `Ctrl+Shift+L` |
| Duplicate selection | `Ctrl+D` |
| Previous paragraph (Shift extends selection) | `Ctrl+[` |
| Next paragraph (Shift extends selection) | `Ctrl+]` |
| Previous word (Shift extends selection) | `Ctrl+Left` |
| Next word (Shift extends selection) | `Ctrl+Right` |
| Previous word part (Shift extends selection) | `Ctrl+/` |
| Next word part (Shift extends selection) | `Ctrl+\` |
| Rectangular block selection | `Alt+Shift+Movement` |
| Extend rectangular selection to start of line | `Alt+Shift+Home` |
| Extend rectangular selection to end of line | `Alt+Shift+End` |
| Go to line… | `F2` |
| Zoom in | `F3` |
| Zoom out | `Ctrl+F3` |

---

## Code Folding

Fold-point markers in the fold margin:

- **Click** — expand or collapse that fold without affecting its children
- **Ctrl+Click** — toggle the fold and apply the same operation recursively to all children
- **Shift+Click** — expand all child folds without toggling the clicked fold itself
- **Ctrl+Shift+Click anywhere in the fold margin** — expand or contract all top-level folds simultaneously; also available via **View → Toggle all folds**

> **Tip:** To open a large block with all grandchildren still folded, first collapse it with
> `Ctrl+Click`, then expand with a plain click.

---

## Find and Replace

**Find (`Ctrl+F` or Search → Find…)**

Options:
- **Direction** — upward or downward
- **Match whole word only** — matches only when surrounded by spaces, tabs, punctuation, or line boundaries
- **Match case** — case-sensitive search

**Find and Replace (`Ctrl+R` or Search → Find and Replace…)**

Adds:
- **Replace** — replace current match and advance
- **Replace All** — replace every occurrence
- Regular-expression replacement is supported

---

## Bookmarks

| Menu command | Shortcut | Description |
|--------------|----------|-------------|
| Add/Remove marker | `Ctrl+F7` | Toggle a bookmark on the current line |
| Find next marker | `Ctrl+F8` | Jump to the next bookmark |
| Find previous marker | `Ctrl+F9` | Jump to the previous bookmark |
| Remove all markers | — | Clear every bookmark in the current document |

---

## Auto-Completion

Press `F12` to open a pop-up list of nuBASIC keywords matching the characters typed so far.
Navigate with the arrow keys; press `Tab` to insert.

---

## Toolbar

| Button | Menu item | Shortcut |
|--------|-----------|----------|
| New | File → New | `Ctrl+N` |
| Open | File → Open… | `Ctrl+O` |
| Save | File → Save | `Ctrl+S` |
| Debug | Debug → Start Debugging | `F5` |
| Stop | Debug → Stop Debugging | `Esc` |
| Breakpoint | Debug → Toggle Breakpoint | `F9` |
| Build | Debug → Build Program | `Ctrl+B` |
| Evaluate | Debug → Evaluate Selection | — |
| Step Into | Debug → Step Into | `F11` |
| Step Over | Debug → Step Over | `F10` |
| Step Out | Debug → Step Out | `Shift+F11` |
| Cont | Debug → Continue Debugging | `F8` |
| Find | Search → Find… | `Ctrl+F` |
| Con Top | Debug → Console Window Topmost | — |
| Ide Top | Debug → IDE Window Topmost | — |

---

## Context-Sensitive Help and Online Help

**Context-sensitive help (`F1`):** Select a keyword in the editor and press `F1`. The IDE
displays the full built-in help entry in the output panel — same as `Help <keyword>` in
the REPL.

**Online help (`Ctrl+F1`):** With a keyword selected, opens a search query for that keyword
in the default web browser.

---

## Integrated Debugger

Breakpoints appear as red circles in the left margin; the program-counter arrow marks the
next line to execute.

### Debug Menu

| Command | Shortcut | Description |
|---------|----------|-------------|
| Build Program | `Ctrl+B` | Compile program metadata. Runs automatically on first start and after source modifications. |
| Start Debugging | `F5` | Launch program with debugger attached. |
| Stop Debugging | `Esc` | Terminate the running program immediately. |
| Continue Debugging | `F8` | Resume from the current breakpoint. |
| Step Into | `F11` | Execute the current line. Descends into called `Sub` or `Function`. |
| Step Over | `F10` | Execute the current line as a single unit. Calls run to completion; halts at the next line of the *caller*. |
| Step Out | `Shift+F11` | Resume until the current `Sub`/`Function` returns. |
| Run to Cursor | `Ctrl+F10` | Resume until the line under the cursor is reached, then halt. |
| Pause / Break | `Ctrl+Pause` | Interrupt a running program at the next steppable statement. |
| Evaluate Selection (Data-Tips) | — | Evaluate the selected expression and display the result as an inline annotation. |
| Start Without Debugging | `Ctrl+F5` | Run without debugger. Press `Ctrl+C` to interrupt. |
| Toggle Breakpoint | `F9` | Add or remove a breakpoint on the current line. |
| Delete All Breakpoints | — | Remove every breakpoint in the current document. |
| Go to Program Counter | — | Scroll to the line that will execute next. |
| Go to Procedure | — | Open a sub-menu listing every Sub and Function; select to jump to its entry point. |
| Console Window Topmost | — | Keep the nuBASIC console window above all other windows. |
| IDE Window Topmost | — | Return normal z-order and bring the IDE to the foreground. |

### Output Panel

The bottom of the IDE contains a tabbed panel with two tabs:

- **Output** — interpreter messages, build results, and Data-Tips evaluation results. Color-coded: white (normal), yellow (warnings), red (errors).
- **Console** — the embedded nuBASIC graphical console. All `Print` output, graphics, and keyboard/mouse input appear here.

---

## Multi-source projects

Programs longer than a single file can be organised as a project.
A nuBASIC project is described by a small text manifest with the
`.nbp` extension that records the entry-point script, the syntax
mode, and a display name. The included files are still brought in
by `Include` / `#Include` directives in the entry script — `.nbp`
only tells the IDE which file is the entry point.

```text
name = MyProgram
syntax = modern
entry = src/main.bas
```

| Key | Meaning |
|-----|---------|
| `name` | Display name shown in the title bar and in the recent-projects list |
| `syntax` | `legacy` or `modern` — initial `Syntax` mode for the entry file |
| `entry` | Path to the entry-point `.bas` file, relative to the directory of the `.nbp` |

| Menu command | Description |
|--------------|-------------|
| **File → Open Project…** | Pick a `.nbp` file. The IDE loads the entry script, sets the declared syntax mode, and updates the title bar. |
| **File → Close Project** | Detach the current project; the editor reverts to single-file mode. |
| **File → Recent Projects** | Lists recently opened `.nbp` files. |

Once a project is open, **Build / Start Debugging** uses the entry
script as the program entry point. The debugger walks transparently
into `Include`-d files; breakpoints in any of them are honoured, and
the call stack panel shows the file name and line number for every
frame. From the CLI, the same `.nbp` can be passed to `nubasic -e`
or `nubasicdebug` to run the project as a unit.

---

## VS Code Extension

A VS Code extension ships alongside the native IDE and provides
syntax highlighting, a Run button, and a debug adapter compatible
with the standard VS Code debug UI (breakpoints, Step Into / Over /
Out, watch, call stack, variables).

**Installation.** The Windows MSI's optional `VSCodeExtension`
component installs the extension automatically. Manual install:

```
code --install-extension <nubasic>/bin/nubasic-latest.vsix
```

**Running and debugging.** Open a `.bas` (or `.nbp`) file and press
**F5**. The extension creates a default `launch.json` of type
`nubasic`. Available modes:

| `request` | Effect |
|-----------|--------|
| `launch` | Run with the debugger attached |
| `launch` + `noDebug: true` | Run without breaking on breakpoints |
| `launch` + `mode: "graphics"` | Hybrid `--graphics-window` mode: graphics in a separate GDI window, debugger and stdout in the VS Code session |

The extension picks the interpreter from
`nubasic.executablePath`, the `NUBASIC_HOME` environment variable,
`%PATH%`, or the `HKCU\Software\nuBASIC\InstallDir` registry key.
It launches `nubasicdebug` for debug sessions and `nubasic` for
plain Run sessions.

---

← [Command Reference](Command-Reference) | Next: [History and Evolution](History-and-Evolution)
