# Microsoft Store listing texts

Texts to paste into Partner Center for the nuBASIC MSIX submission.
Section and field names follow Partner Center. Limits are noted next to each
field; Description must be plain text, with no HTML or URLs.

## Properties

| Field | Value |
| --- | --- |
| Category | Developer tools |
| Privacy policy URL | https://github.com/eantcal/nubasic/blob/main/PRIVACY.md |
| Website | https://github.com/eantcal/nubasic |
| Support contact info | https://github.com/eantcal/nubasic/issues |

## Submission options: Restricted capabilities

Capability: `runFullTrust`

```text
nuBASIC is a classic Win32 desktop application: a BASIC interpreter with a
native IDE and debugger, packaged as MSIX with the Desktop Bridge. It is not
a UWP app, so it requires runFullTrust to run at all.

The capability is used to:
- run the IDE and the nubasic.exe command-line interpreter (exposed through
  an app execution alias) as regular desktop processes;
- open, edit and save BASIC source files (.bas) and projects (.nbp) chosen by
  the user anywhere on disk, and run them;
- open a console window for program output and draw graphics with GDI;
- optionally call functions in native libraries (Declare Function ... Lib),
  a documented language feature that the user can disable with
  --disable-native-calls.

nuBASIC does not collect or transmit any data. It only accesses files and
system functions when the user's own BASIC program asks for them.
```

## Store listing (English)

### Product name

nuBASIC

### Short description

Limit 1000 characters; the first 270 are shown without expanding.

```text
A modern, open-source BASIC interpreter with a native IDE and debugger. Write classic line-numbered BASIC, structured code or object-oriented programs, draw graphics, and build retro first-person games with the built-in nuRCADE raycasting engine.
```

### Description

Limit 10,000 characters.

```text
nuBASIC is a modern, open-source BASIC interpreter for Windows. It is a friendly first programming language for beginners and a tribute to the home-computer era for retro-computing fans, while staying capable enough for real programs.

THREE STYLES IN ONE LANGUAGE
- Classic BASIC: line numbers, GoTo and GoSub, and an immediate-mode prompt where every statement runs as soon as you type it.
- Structured BASIC: Sub and Function with ByRef and ByVal parameters, If/ElIf/Else, For, While, Do...Loop, Select Case, Struct, Const, multi-file programs with Include, and a main() entry point with command-line arguments.
- Object-oriented BASIC: classes with fields and methods, constructors and destructors, single inheritance, virtual methods, static methods, and Public/Protected/Private access control.
All three styles can be mixed in the same program.

A COMPLETE DEVELOPMENT ENVIRONMENT
- Native IDE with syntax highlighting, auto-completion, code folding and bookmarks.
- Integrated debugger: breakpoints, Step Into, Step Over, Step Out, Run to Cursor, Pause, watch window and call stack.
- Multi-file projects (.nbp) and file associations for .bas and .nbp files.
- The nubasic command-line interpreter, available from any terminal.
- Built-in help for every keyword.

GRAPHICS AND GAMES
- Lines, rectangles, ellipses, filled shapes, text, bitmaps and pixel access.
- Flicker-free double-buffered animation.
- Mouse and keyboard input.
- nuRCADE, an integrated raycasting engine for pseudo-3D first-person scenes in the style of 90s arcade shooters: textured walls, sprites, player movement and collision detection, all driven from simple BASIC functions.

LANGUAGE AND RUNTIME
- Integer, Double, Boolean, Long64, String, Byte and user-defined types.
- Built-in arrays and hash tables.
- Sequential, binary and random-access file I/O.
- UTF-8 strings and console output.
- Optional calls to native Windows libraries, which can be turned off.

Ready-to-run examples are included: games, fractals, animations, graphics demos and a complete raycasting demo.

nuBASIC is free and open source under the MIT License. It does not collect any data.
```

### Product features

Up to 20 items, 200 characters each, no bullets.

```text
Classic, structured and object-oriented BASIC in one language
Immediate-mode prompt: type a statement and it runs at once
Native IDE with syntax highlighting, auto-completion and code folding
Integrated debugger with breakpoints, stepping, watch and call stack
2D graphics, bitmaps and flicker-free animation
nuRCADE raycasting engine for retro first-person games
Built-in arrays, hash tables, structs and classes
Multi-file projects and .bas/.nbp file associations
Command-line interpreter available from any terminal
Ready-to-run example games, fractals and demos
Free and open source (MIT License), no data collection
```

### Keywords

```text
BASIC
programming
interpreter
IDE
retro
learn to code
raycasting
```

### Copyright and trademark info

```text
Copyright (c) Antonino Calderone. Released under the MIT License.
```

### Screenshots

At least one is required; Microsoft recommends four. Suggested shots:

1. IDE with a structured or object-oriented example open, syntax highlighting visible.
2. Debugger stopped at a breakpoint, with the watch and call stack panes.
3. A graphics example running (for example a fractal or `plane.bas`).
4. The nuRCADE raycasting demo (`examples/raycast/eclipse_protocol.bas`).

## Store listing (Italiano)

The package declares `it-it`, so Partner Center requires a complete Italian
listing as well. Screenshots can be the same as the English listing.

### Product name

nuBASIC

### Short description

```text
Un interprete BASIC moderno e open source, con IDE e debugger nativi. Scrivi in BASIC classico con i numeri di riga, in modo strutturato o a oggetti, disegna grafica e crea giochi retro in prima persona con il motore di raycasting nuRCADE integrato.
```

### Description

```text
nuBASIC è un interprete BASIC moderno e open source per Windows. È un primo linguaggio di programmazione semplice per chi inizia e un omaggio all'epoca degli home computer per gli appassionati di retro-computing, pur restando adatto a programmi veri.

TRE STILI IN UN SOLO LINGUAGGIO
- BASIC classico: numeri di riga, GoTo e GoSub, e un prompt in modalità immediata in cui ogni istruzione viene eseguita appena la scrivi.
- BASIC strutturato: Sub e Function con parametri ByRef e ByVal, If/ElIf/Else, For, While, Do...Loop, Select Case, Struct, Const, programmi su più file con Include e un punto di ingresso main() con argomenti da riga di comando.
- BASIC a oggetti: classi con campi e metodi, costruttori e distruttori, ereditarietà singola, metodi virtuali, metodi statici e controllo di accesso Public/Protected/Private.
I tre stili possono essere combinati nello stesso programma.

UN AMBIENTE DI SVILUPPO COMPLETO
- IDE nativo con evidenziazione della sintassi, completamento automatico, code folding e segnalibri.
- Debugger integrato: breakpoint, Step Into, Step Over, Step Out, Run to Cursor, pausa, finestra di watch e call stack.
- Progetti su più file (.nbp) e associazione dei file .bas e .nbp.
- L'interprete nubasic da riga di comando, disponibile da qualsiasi terminale.
- Guida integrata per ogni parola chiave.

GRAFICA E GIOCHI
- Linee, rettangoli, ellissi, forme piene, testo, bitmap e accesso ai singoli pixel.
- Animazioni senza sfarfallio con doppio buffer.
- Input da mouse e tastiera.
- nuRCADE, un motore di raycasting integrato per scene pseudo-3D in prima persona nello stile degli sparatutto arcade anni '90: pareti con texture, sprite, movimento del giocatore e rilevamento delle collisioni, tutto controllato da semplici funzioni BASIC.

LINGUAGGIO E RUNTIME
- Tipi Integer, Double, Boolean, Long64, String, Byte e tipi definiti dall'utente.
- Array e tabelle hash integrati.
- I/O su file sequenziale, binario e ad accesso casuale.
- Stringhe e output su console in UTF-8.
- Chiamate opzionali a librerie native di Windows, disattivabili.

Sono inclusi esempi pronti da eseguire: giochi, frattali, animazioni, demo grafiche e una demo completa di raycasting.

nuBASIC è gratuito e open source con licenza MIT. Non raccoglie alcun dato.
```

### Product features

```text
BASIC classico, strutturato e a oggetti in un solo linguaggio
Prompt in modalità immediata: scrivi un'istruzione e viene eseguita subito
IDE nativo con evidenziazione della sintassi, completamento automatico e code folding
Debugger integrato con breakpoint, esecuzione passo passo, watch e call stack
Grafica 2D, bitmap e animazioni senza sfarfallio
Motore di raycasting nuRCADE per giochi retro in prima persona
Array, tabelle hash, struct e classi integrati
Progetti su più file e associazione dei file .bas/.nbp
Interprete da riga di comando disponibile da qualsiasi terminale
Esempi pronti da eseguire: giochi, frattali e demo
Gratuito e open source (licenza MIT), nessuna raccolta di dati
```

### Keywords

```text
BASIC
programmazione
interprete
IDE
retro
imparare a programmare
raycasting
```

### Copyright and trademark info

```text
Copyright (c) Antonino Calderone. Rilasciato con licenza MIT.
```
