# Integrated Development Environment (IDE)

← [Command Reference](Command-Reference) | Next: [History and Evolution](History-and-Evolution)

---

The nuBASIC IDE bundles an advanced syntax highlighting editor, an interpreter, and a debugger
into a single application available for Windows and Linux (GTK+2). It is the recommended tool
for writing non-trivial programs.

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
- **Integrated debugger** — breakpoints, step-by-step execution, expression evaluation (data-tips)
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
| Evaluate | Debug → Evaluate Selection | `F11` |
| Step | Debug → Step | `F10` |
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
| Step | `F10` | Execute the current line and halt at the next. Enters called routines. |
| Evaluate Selection (Data-Tips) | `F11` | Evaluate the selected expression and display the result as an inline annotation. |
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

← [Command Reference](Command-Reference) | Next: [History and Evolution](History-and-Evolution)
