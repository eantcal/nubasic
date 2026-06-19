# nuBASIC — Guida Utente

> Versione 2.0 · http://www.nubasic.eu/
> Autore: Antonino Calderone — antonino.calderone@gmail.com

---

## Indice dei contenuti

1. [BASIC: Un linguaggio nato per imparare](#1-basic-un-linguaggio-nato-per-imparare)
2. [Introduzione a nuBASIC](#2-introduzione-a-nubasic)
3. [Primi passi](#3-primi-passi)
4. [Il linguaggio nuBASIC](#4-il-linguaggio-nubasic)
   - 4.1 [Variabili e tipi](#41-variabili-e-tipi)
   - 4.2 [Operatori](#42-operatori)
   - 4.3 [Controllo del flusso](#43-controllo-del-flusso)
   - 4.4 [Subroutine e funzioni](#44-subroutine-e-funzioni)
   - 4.5 [Classi e oggetti](#45-classi-e-oggetti)
   - 4.6 [Punto di ingresso main()](#46-punto-di-ingresso-main)
   - 4.7 [Moduli con namespace](#47-moduli-con-namespace)
   - 4.8 [Strutture](#48-strutture)
   - 4.9 [Array](#49-array)
   - 4.10 [Tabelle hash](#410-tabelle-hash)
   - 4.11 [I/O su file](#411-io-su-file)
   - 4.12 [DATA, READ, RESTORE](#412-data-read-restore)
   - 4.13 [Gestione delle stringhe](#413-gestione-delle-stringhe)
   - 4.14 [Chiamate a librerie native](#414-chiamate-a-librerie-native)
   - 4.15 [Continuazione di riga](#415-continuazione-di-riga)
5. [Grafica e multimedia](#5-grafica-e-multimedia)
   - 5.1 [Primitive di disegno](#51-primitive-di-disegno)
   - 5.2 [Rendering senza sfarfallio — ScreenLock / ScreenUnlock / Refresh](#52-rendering-senza-sfarfallio)
   - 5.3 [Input del mouse](#53-input-del-mouse)
   - 5.4 [Audio e gestione della finestra](#54-audio-e-gestione-della-finestra)
   - 5.5 [Pseudo-3D con WinRayCast (Windows)](#55-pseudo-3d-con-winraycast)
6. [Riferimento ai comandi](#6-riferimento-ai-comandi)
   - 6.1 [Comandi della console (REPL / CLI)](#61-comandi-della-console)
   - 6.2 [Istruzioni](#62-istruzioni)
   - 6.3 [Funzioni integrate](#63-funzioni-integrate)
   - 6.4 [Riferimento agli operatori](#64-riferimento-agli-operatori)
7. [Ambiente di sviluppo integrato (IDE)](#7-ambiente-di-sviluppo-integrato-ide)
   - 7.1 [Editor con evidenziazione della sintassi](#71-editor-con-evidenziazione-della-sintassi)
   - 7.2 [Comandi da tastiera](#72-comandi-da-tastiera)
   - 7.3 [Raggruppamento del codice](#73-raggruppamento-del-codice)
   - 7.4 [Trova e sostituisci](#74-trova-e-sostituisci)
   - 7.5 [Segnalibri](#75-segnalibri)
   - 7.6 [Completamento automatico](#76-completamento-automatico)
   - 7.7 [Barra degli strumenti](#77-barra-degli-strumenti)
   - 7.8 [Guida contestuale e guida in linea](#78-guida-contestuale-e-guida-in-linea)
   - 7.9 [Debugger integrato](#79-debugger-integrato)
   - 7.10 [Estensione VS Code](#710-estensione-vs-code)
   - 7.11 [Progetti multi-sorgente](#711-progetti-multi-sorgente)
8. [Storia ed evoluzione di nuBASIC](#8-storia-ed-evoluzione-di-nubasic)
9. [Come è fatto nuBASIC: Internals dell'interprete](#9-come-è-fatto-nubasic-internals-dellinterprete)
   - 9.1 [Componenti principali](#91-componenti-principali)
   - 9.2 [Un interprete orientato alle righe](#92-un-interprete-orientato-alle-righe)
   - 9.3 [Token e il tokenizzatore](#93-token-e-il-tokenizzatore)
   - 9.4 [Contenitore della lista di token](#94-contenitore-della-lista-di-token)
   - 9.5 [Analisi del codice](#95-analisi-del-codice)
   - 9.6 [Analisi delle espressioni in dettaglio](#96-analisi-delle-espressioni-in-dettaglio)
   - 9.7 [Il tipo Variant](#97-il-tipo-variant)
   - 9.8 [Tracciare l'esecuzione di un programma semplice](#98-tracciare-lesecuzione-di-un-programma-semplice)
   - 9.9 [Estendere il set di funzioni integrate](#99-estendere-il-set-di-funzioni-integrate)
10. [Compilare nuBASIC dai sorgenti](#10-compilare-nubasic-dai-sorgenti)
    - 10.1 [Ottenere i sorgenti](#101-ottenere-i-sorgenti)
    - 10.2 [Panoramica degli obiettivi di compilazione](#102-panoramica-degli-obiettivi-di-compilazione)
    - 10.3 [Compilazione su Windows](#103-compilazione-su-windows)
    - 10.4 [Compilazione su Linux](#104-compilazione-su-linux)
    - 10.5 [Compilazione su macOS](#105-build-su-macos)
    - 10.6 [Compilazione su iOS con iSH](#106-compilazione-su-ios-con-ish)
    - 10.7 [Riferimento alle opzioni CMake](#107-riferimento-alle-opzioni-cmake)
    - 10.8 [Creazione di programmi di installazione e pacchetti](#108-creazione-di-installer-e-pacchetti)
      - MSI Windows (target Visual Studio `CreateInstaller` o riga di comando)
      - Ubuntu / Debian `.deb` (`create_ubuntu_package.sh` o cpack manuale)
      - macOS installazione manuale

---

## 1. BASIC: Un linguaggio nato per imparare

### Origini

BASIC — **B**eginner's **A**ll-purpose **S**ymbolic **I**nstruction **C**ode — fu progettato nel 1964
al Dartmouth College da John G. Kemeny e Thomas E. Kurtz. Il loro obiettivo era semplice e rivoluzionario
per l'epoca: dare agli studenti privi di qualsiasi esperienza di programmazione un linguaggio che
potessero imparare e usare produttivamente in una sola sessione. Ogni scelta progettuale fu fatta
pensando a quel principiante.

A differenza di Fortran o COBOL, che richiedevano passaggi separati di compilazione e la conoscenza
dei linguaggi di controllo dei lavori, BASIC funzionava in modo interattivo. Si digitava una riga,
si premeva Invio, si otteneva un risultato. Si numeravano le righe, si digitava `RUN` e il programma
veniva eseguito immediatamente. Non c'era alcuna barriera tra il pensiero e l'esecuzione.

### L'era dell'informatica casalinga

BASIC divenne il linguaggio di programmazione dominante della rivoluzione del personal computer
alla fine degli anni '70 e nei primi anni '80. Quasi ogni computer domestico — l'Apple II,
il Commodore 64, lo Sinclair Spectrum, il TRS-80, il BBC Micro — era fornito con un interprete
BASIC integrato nella ROM. Per un'intera generazione, BASIC non era semplicemente un linguaggio
di programmazione; era *il* modo di interagire con un computer al di là dell'esecuzione di
software preconfezionato.

I programmi erano brevi, lo schermo era immediatamente visibile e il ciclo di feedback era
istantaneo. Un bambino di dieci anni poteva scrivere un gioco. Uno scienziato poteva scrivere
una simulazione. BASIC ha democratizzato la programmazione molto prima che esistesse il web.

Il dialetto classico di quell'epoca aveva uno stile inconfondibile:

```basic
10 PRINT "ENTER YOUR NAME:"
20 INPUT N$
30 PRINT "HELLO, "; N$; "!"
40 GOTO 10
```

Ogni riga aveva un numero, il flusso era controllato con `GOTO` e `GOSUB`, e le variabili erano corte.
I numeri di riga servivano a un doppio scopo: erano gli indirizzi usati da `GOTO` e definivano
l'ordine in cui il programma era memorizzato ed elencato.

### Dialetti ed evoluzione

BASIC non fu mai un linguaggio standardizzato e unico. Ogni produttore di hardware scrisse il
proprio interprete e i dialetti si moltiplicarono. Il BASIC di Microsoft (poi GW-BASIC e
QuickBASIC) divenne il più diffuso sui PC compatibili IBM. Turbo Basic, PowerBASIC e infine
Visual Basic seguirono, aggiungendo ciascuno funzionalità di programmazione strutturata, sistemi
di tipi migliori e capacità grafiche.

La rivoluzione strutturata nel BASIC arrivò a metà degli anni '80: `WHILE`/`WEND` sostituì i
cicli basati su `GOTO`, `SUB` e `FUNCTION` sostituirono le subroutine numerate a riga, e i
compilatori fecero girare i programmi BASIC a velocità quasi nativa. I successori moderni come
QB64, FreeBASIC e PureBasic portano avanti quella tradizione ancora oggi.

### Cosa condividono tutti i dialetti BASIC

Nonostante la diversità, la maggior parte degli interpreti BASIC condivide un nucleo riconoscibile:

- Parole chiave non sensibili alle maiuscole (`PRINT`, `Print`, `print` sono tutte uguali)
- Le variabili richiedono poca o nessuna dichiarazione; un carattere suffisso indica spesso il tipo
  (`$` per le stringhe, `%` per gli interi)
- L'istruzione `PRINT` invia l'output allo schermo
- `INPUT` legge dalla tastiera
- `IF … THEN … ELSE` per le decisioni
- `FOR … TO … NEXT` per i cicli contati
- `GOTO` e `GOSUB`/`RETURN` per il controllo del flusso
- I numeri di riga sono accettati ma non sempre richiesti

---

## 2. Introduzione a nuBASIC

nuBASIC è un moderno interprete BASIC open-source scritto in C++20, creato da Antonino
Calderone e rilasciato per la prima volta nel 2014. È profondamente radicato nella tradizione
BASIC — interattivo, accogliente e immediatamente produttivo — pur aggiungendo le funzionalità
di programmazione strutturata e le capacità grafiche che i programmi contemporanei richiedono.

### nuBASIC come dialetto BASIC

nuBASIC appartiene alla famiglia dei dialetti BASIC strutturati discendenti da QuickBASIC e
dai suoi successori. Come loro, accetta sia i programmi in stile classico con numeri di riga
sia i programmi in stile moderno che si affidano interamente a etichette, `Sub`, `Function`
e strutture di controllo moderne. I due stili possono persino essere mescolati nello stesso file.

Ciò che nuBASIC aggiunge rispetto alla classica base BASIC:

- **Sistema di tipi ricco** — Integer (`%`), Double (senza suffisso), Boolean (`#`), Long64 (`&`),
  String (`$`), Byte (`@`), `Any` (tipo dedotto), e tipi `Struct` definiti dall'utente
- **Controllo del flusso strutturato** — `If/ElIf/Else/End If`, `While/Wend`, `Do/Loop While`,
  `Exit For/While/Do/Sub/Function`
- **Subroutine e funzioni di prima classe** — con parametri tipizzati, valori di ritorno tipizzati
  e ricorsione
- **Array e tabelle hash** integrati nel linguaggio
- **I/O su file completo** — sequenziale, binario e ad accesso casuale
- **Grafica** — linee, rettangoli, ellissi, forme piene, accesso ai pixel, bitmap, testo
- **Rendering con doppio buffer** — `ScreenLock`/`ScreenUnlock`/`Refresh` per animazioni senza sfarfallio
- **Input del mouse** — posizione e stato dei pulsanti leggibili dal codice BASIC
- **UTF-8** per i letterali stringa e l'output della console, incluse sequenze di escape per caratteri Unicode
- **Guida interattiva integrata** — `Help keyword` e `Apropos topic` dal REPL

### Piattaforme

La versione Windows fornisce una finestra console dedicata basata su GDI che renderizza testo e
grafica nativamente, funziona sia autonomamente sia incorporata nell'IDE di nuBASIC, e supporta
tutte le API grafiche incluso il disegno a livello di pixel. La versione Linux usa il terminale
per il testo e X11 per la grafica. È disponibile anche una variante *console* (senza grafica, audio
o supporto per il gestore di finestre) per ambienti con risorse limitate.

### La modalità console e la build console

#### Modalità console

Quando nuBASIC si avvia senza un argomento file, entra in **modalità console** — il ciclo
interattivo Read-Eval-Print (REPL). Questo è l'ambiente di lavoro principale per esplorare il
linguaggio, scrivere e testare programmi brevi e fare il debug di quelli più lunghi.

In modalità console, nuBASIC accetta due tipi di input:

- **Istruzioni immediate** — digitate senza un numero di riga, eseguite nel momento in cui si
  preme Invio. I risultati appaiono immediatamente. Questo è il modo più veloce per sperimentare
  con una funzione o testare un calcolo:

  ```
  nubasic> Print Sin(PI() / 6)
   0.5
  nubasic> x% = 42 : Print x% * x%
   1764
  nubasic> Help Mid$
  ```

- **Righe numerate** — le righe precedute da un intero vengono memorizzate nel buffer del
  programma invece di essere eseguite. Una volta inserite abbastanza righe, digitate `Run` per
  eseguire il programma, `List` per rivederlo, `Save` per salvarlo su file, oppure `New` per
  cancellarlo e ricominciare:

  ```
  nubasic> 10 For i%=1 To 5
  nubasic> 20   Print i% * i%
  nubasic> 30 Next i%
  nubasic> Run
   1
   4
   9
   16
   25
  ```

I due modi coesistono naturalmente: potete caricare un file con `Load`, ispezionarlo con `List`,
impostare un punto di interruzione con `Break`, eseguirlo con `Run` e poi esaminare i valori
delle variabili con `Vars` — il tutto senza uscire dall'interprete.

Il set di comandi della modalità console include tutto il necessario per un ciclo completo di
modifica-esecuzione-debug:

| Categoria | Comandi |
|----------|----------|
| Esecuzione | `Run`, `Cont`, `Resume`, `End` |
| Modifica | `List`, `New`, `Clr`, `Renum`, `Grep` |
| File | `Load`, `Save`, `Exec`, `Pwd`, `Cd` |
| Debug | `Break`, `RmBrk`, `ClrBrk`, `TrOn`, `TrOff`, `StOn`, `StOff`, `Vars`, `Stop` |
| Guida | `Help`, `Apropos`, `Ver`, `Meta` |
| Sistema | `!` (shell), `Exit` |

Su Windows, la console è renderizzata da una finestra GDI dedicata che supporta sia l'output
testuale sia la grafica a livello di pixel sulla stessa superficie. La finestra può essere
spostata e ridimensionata dall'interno di un programma usando `MoveWindow`. Su Linux, il
terminale gestisce l'I/O testuale mentre una finestra X11 fornisce la superficie grafica quando
vengono usate istruzioni grafiche.

#### La build console

La **build console** è una variante semplificata di nuBASIC compilata senza supporto per grafica,
audio, mouse o gestore di finestre. Tutte le istruzioni grafiche (`Line`, `FillRect`, `TextOut`,
`ScreenLock`, `PlotImage`, …) e le relative funzioni (`GetMouseX`, `GetSWidth`,
`MoveWindow`, `PlaySound`, `MsgBox`, …) sono assenti.

Ciò che rimane è un interprete testuale completamente funzionale: il nucleo completo del linguaggio,
tutte le funzioni per stringhe e matematica, I/O su file, tabelle hash, `Sub`/`Function`, `Struct`
e la modalità console completa con tutti i comandi di debug. I programmi che non usano la grafica
funzionano in modo identico su entrambe le versioni.

La build console è la scelta giusta quando:

- Si esegue su un **server headless** o un sistema embedded senza display
- Si effettua **scripting** per operazioni di elaborazione testi o manipolazione file dove la grafica è irrilevante
- Si **insegnano** concetti di programmazione pura senza la distrazione di un ambiente grafico
- Si è in **ambienti con risorse limitate** dove le dimensioni ridotte del binario o le dipendenze ridotte sono importanti

Per verificare in fase di esecuzione quale versione è attiva, usate `GetPlatId()` (restituisce 1 su
Windows, 2 su Linux) e verificate la presenza di una funzione grafica prima di chiamarla — o
semplicemente progettate i programmi in modo che non usino la grafica quando è richiesta la
portabilità tra entrambe le versioni.

### Perché nuBASIC?

Due qualità rendono nuBASIC una scelta valida rispetto ad altre opzioni, e queste tirano in
direzioni opposte in un modo che di solito è difficile da conciliare.

**È genuinamente semplice.** Un principiante completo senza alcuna esperienza di programmazione
può scrivere ed eseguire programmi in pochi minuti. Non c'è nessun sistema di progetto da
configurare, nessun compilatore da invocare, nessun runtime da installare separatamente. Si digita
una riga e si preme Invio. Il REPL interattivo fornisce un feedback immediato. I messaggi di
errore sono in linguaggio chiaro. Il sistema di guida integrato è sempre a un solo comando di
distanza. Queste non sono caratteristiche aggiuntive — sono la filosofia progettuale di base
ereditata direttamente dalla tradizione BASIC.

**È anche un linguaggio completo.** Strutture, parametri tipizzati, ricorsione, tabelle hash,
I/O su file, grafica e animazione con doppio buffer sono tutti disponibili quando ne avete bisogno.
Un programma non supera le capacità di nuBASIC solo perché diventa più ambizioso. Lo stesso
interprete che esegue il primo `Print "Hello"` di un principiante esegue anche un ray-caster 3D,
un clone di Tetris e un renderer dell'insieme di Mandelbrot.

Il terzo motivo è la flessibilità di stile. nuBASIC accetta sia programmi classici numerati a riga
sia codice strutturato moderno, e consente ai due di coesistere nello stesso file. Questo non è
semplicemente una concessione alla compatibilità — significa che un principiante può iniziare con
la sintassi più semplice possibile e adottare gradualmente costrutti strutturati man mano che la
sua comprensione cresce, senza mai dover reimparare il linguaggio o cambiare strumenti.

L'insieme di Mandelbrot (noto come la "pietra di Rosetta" della geometria frattale, perché ogni
linguaggio lo implementa come dimostrazione) illustra perfettamente questo. Ecco lo stesso calcolo
scritto in entrambi gli stili, entrambi dei quali vengono eseguiti correttamente su nuBASIC:

**Stile classico — numeri di riga, GoTo, espressioni inline:**

```basic
5   Rem rosetta_classic.bas
10  For x0 = -2 To 2 Step .013
20  For y0 = -1.5 To 1.5 Step .013
30  x = 0 : y = 0 : iteration = 0 : maxIteration = 223
70  xtemp = x*x - y*y + x0
80  y = 2*x*y + y0
90  x = xtemp : iteration = iteration + 1
110 If ((x*x + y*y <= 4) And (iteration < maxIteration)) Then GoTo 70
120 If iteration <> maxIteration Then c = iteration Else c = 0
130 d% = 150 : dx% = 300 : dy% = 300
140 FillRect x0*d%+dx%, y0*d%+dy%, x0*d%+dx%+2, y0*d%+dy%+2, Int(c)*16
150 Next y0
160 Next x0
```

**Stile strutturato — etichette, While, If/End If, variabili con nome:**

```basic
' rosetta_structured.bas
For x0 = -2 To 2 Step .013
   ScreenLock
   For y0 = -1.5 To 1.5 Step .013
      x = 0 : y = 0 : iteration = 0 : maxIteration = 223

      While ((x*x + y*y <= 4) And (iteration < maxIteration))
         xtemp = x*x - y*y + x0
         y = 2*x*y + y0
         x = xtemp
         ++iteration
      End While

      If iteration <> maxIteration Then c = iteration Else c = 0
      FillRect x0*150+300, y0*150+300, x0*150+302, y0*150+302, Int(c)*16
   Next y0
   ScreenUnlock
Next x0
```

La versione strutturata è più facile da leggere e mantenere; la versione classica è più compatta
e più vicina a come l'algoritmo potrebbe apparire in un libro di testo degli anni '80. nuBASIC
esegue entrambe senza modifiche.

#### Tre stili in un unico linguaggio

nuBASIC supporta **tre paradigmi di programmazione nello stesso interprete**, piu' un modo
per uscire dal linguaggio e raggiungere il codice nativo. Un singolo programma puo'
mescolarli tutti.

1. **BASIC classico** — righe numerate, REPL in modalita' immediata, `GoTo` / `GoSub` /
   `On … GoTo`. E' il dialetto della versione "classica" del Mandelbrot qui sopra: era
   pensato perche' un principiante potesse scrivere poche righe e vederle eseguite
   subito, senza compilatore, senza progetto, senza boilerplate. nuBASIC mantiene questa
   esperienza: digitare `Print "Ciao"` al prompt stampa `Ciao`, fine.

2. **BASIC strutturato** — `Sub` e `Function` con parametri `ByRef` / `ByVal`, controllo
   di flusso strutturato (`If/ElIf/Else`, `For`, `While`, `Do…Loop While`, `Select Case`,
   `Exit For`/`While`/`Do`/`Sub`/`Function`), `Struct` per i tipi composti, `Const` per i
   valori immutabili, `Include` / `#Include` per dividere il codice su piu' file, e un
   punto d'ingresso `Function main(argc, argv())` che riceve gli argomenti da riga di
   comando. E' lo stile di QuickBASIC e FreeBASIC: i programmi piu' grandi restano
   manutenibili senza dover lasciare BASIC.

3. **BASIC orientato agli oggetti** — classi complete con campi e metodi di istanza
   (`Me`), metodi `Static` a livello di classe, ereditarieta' singola tramite `Inherits`,
   dispatch virtuale (`Overridable` / `Overrides`), dispatch esplicito alla classe base
   tramite `MyBase`, controllo d'accesso a tre livelli (`Public` / `Protected` /
   `Private`), costruttori (`Sub New`) e distruttori RAII (`Sub Delete`) chiamati
   automaticamente quando un oggetto posseduto esce dal proprio scope. Vedi la
   [§4.5](#45-classi-e-oggetti) per la trattazione completa.

   ```basic
   Class Forma
      Overridable Function Descrivi$() As String
         Descrivi$ = "una forma"
      End Function
   End Class

   Class Cerchio
      Inherits Forma
      Public raggio As Double
      Overrides Function Descrivi$() As String
         Descrivi$ = MyBase.Descrivi$() + " (raggio " + Str$(Me.raggio) + ")"
      End Function
   End Class

   Dim c As New Cerchio
   c.raggio = 2.5
   Print c.Descrivi$()    ' una forma (raggio 2.5)
   ```

4. **Librerie native esterne** — quando la libreria standard non basta, uno script puo'
   dichiarare e chiamare funzioni esportate da una qualsiasi libreria condivisa nativa.
   nuBASIC carica la libreria con `LoadLibraryW` su Windows e `dlopen` su Linux/macOS,
   marshalla gli argomenti e i valori di ritorno tramite libffi, e presenta il risultato
   come una normale funzione BASIC. Win32, libc POSIX e qualunque `.so` o `.dylib`
   costruito dall'utente diventano direttamente raggiungibili da un programma BASIC. Vedi
   [§4.14](#414-chiamate-a-librerie-native).

   ```basic
   Declare Function GetCurrentProcessId Lib "kernel32.dll" () As DWORD
   Print GetCurrentProcessId()
   ```

Il punto e' che questi stili non sono dialetti separati: condividono variabili, regole di
scope, sistema dei tipi, interprete e debugger. Un docente puo' rimanere allo stile 1 con
righe numerate e `GoTo` finche' la classe non e' a suo agio; un hobbista puo' adottare
`Sub` e `Function` quando uno script supera le dieci righe; uno sviluppatore esperto puo'
modellare un intero gioco o una simulazione con le classi e raggiungere il sistema
operativo tramite chiamate native — il tutto senza mai uscire da nuBASIC.

Internamente, nuBASIC è costruito da due componenti complementari che rispecchiano la struttura
di qualsiasi linguaggio ben progettato:

- **Il nucleo del linguaggio** — i costrutti elementari: dichiarazioni di variabili, operatori, controllo
  del flusso (`If`, `For`, `While`, `Do`), definizioni di procedure (`Sub`, `Function`), e supporto
  ai tipi (`Struct`, `Dim`, `Const`).
- **La libreria di funzioni integrate** — le capacità standard di cui i programmi hanno bisogno senza
  dover implementare da zero: matematica (`Sin`, `Cos`, `Sqr`, `Pow`, …), manipolazione delle stringhe
  (`Mid$`, `InStr`, `UCase$`, …), I/O su file, ora di sistema, grafica e input del mouse.

Questa separazione rende nuBASIC facile da estendere e facile da imparare per gradi: prima si
padroneggia il nucleo del linguaggio, poi si esplora la libreria secondo necessità.

### Caratteristiche principali in sintesi

- Programmazione strutturata: `Sub`, `Function`, `For`, `While`, `Do…Loop While`, `If/ElIf/Else`
- Sistema di tipi ricco: Integer, Double, Boolean, Long64, String, Any, `Struct` definito dall'utente
- Array e tabelle hash
- I/O su file completo (sequenziale, binario e ad accesso casuale)
- Grafica: linee, rettangoli, ellissi, testo, bitmap, accesso ai pixel
- Rendering senza sfarfallio con doppio buffer (`ScreenLock` / `ScreenUnlock`)
- Input da mouse e tastiera
- Letterali stringa UTF-8 e output della console
- Guida integrata (`Help`, `Apropos`) accessibile direttamente dal REPL

---

## 3. Primi passi

### Avviare l'interprete

```
nubasic                   # REPL interattivo
nubasic myprogram.bas     # carica ed esegue un file
nubasic -e myprogram.bas  # alternativa: esecuzione dalla riga di comando
```

Il REPL interattivo (Read-Eval-Print Loop) accetta sia istruzioni singole sia programmi completi.
Potete digitare un'istruzione e premere Invio per eseguirla immediatamente, oppure inserire righe
numerate per costruire un programma in memoria e poi digitare `RUN`.

### Flag della riga di comando

| Flag | Effetto |
|------|---------|
| `-e <file>` | Esegue un file sorgente nuBASIC ed esce |
| `-l <file>` | Carica un file sorgente nuBASIC nel REPL (senza eseguirlo) |
| `-t`, `--text-mode` | Modalita' testo/batch (nessuna finestra terminale). Indicata per CI e usi headless |
| `--graphics-window` | Apre l'output grafico in una finestra GDI separata, mantenendo I/O testuale e protocollo debugger su stdout. Usata dalla modalita' di debug ibrida di VS Code per programmi grafici |
| `--machine-interface` | Emette eventi machine-friendly del debugger (una riga per evento, formato `key="value"`) su stdout. Usata dall'adapter VS Code; non destinata all'uso interattivo |
| `--disable-native-calls` | Rifiuta a runtime ogni `Declare Function ... Lib ...` (vedi §4.14) |
| `-nx` | Non apre una nuova finestra terminale (solo Windows; rilevante per il binario GDI) |
| `-h [argomento]`, `-?`, `--help` | Mostra la guida integrata. Con `argomento` stampa l'help di quella parola chiave |
| `-v`, `--version` | Stampa la versione ed esce |

La stessa lista e' disponibile al prompt con `nubasic --help`.

### Il vostro primo programma

Digitate direttamente nel REPL o salvate in un file `.bas`:

```basic
Print "Hello, world!"
```

Con i numeri di riga (stile BASIC classico):

```basic
10 Print "Hello, world!"
20 End
```

I numeri di riga sono completamente opzionali. I programmi nuBASIC moderni li omettono e usano
il controllo del flusso strutturato. I due stili possono coesistere — nuBASIC accetta un misto
di righe numerate e non numerate nello stesso file sorgente.

I commenti iniziano con una virgoletta singola `'` o con la parola chiave `Rem`:

```basic
' Questo è un commento
Rem Anche questo è un commento
Print "Hello"   ' commento inline
```

Più istruzioni possono apparire su una singola riga separate da due punti:

```basic
x% = 5 : y% = 10 : Print x% + y%
```

### Shebang (Linux/macOS)

Rendete un file `.bas` direttamente eseguibile su sistemi Unix-like aggiungendo una riga shebang
come prima riga del file:

```basic
#!/usr/local/bin/nubasic
Print "Hello from a script!"
```

Poi contrassegnate il file come eseguibile: `chmod +x myscript.bas && ./myscript.bas`

### Guida integrata

Il REPL include un sistema di guida integrata completo. Non è mai necessario uscire dall'interprete
per cercare una parola chiave:

```
Help               ' elenca tutte le parole chiave raggruppate per categoria
Help Print         ' descrizione completa e sintassi di Print
Help ScreenLock    ' descrizione dell'istruzione ScreenLock
Help examples      ' elenca i programmi di esempio installati con suggerimenti su come caricarli ed eseguirli
Apropos mouse      ' cerca in tutte le descrizioni della guida la parola "mouse"
```

---

## 4. Il linguaggio nuBASIC

### 4.1 Variabili e tipi

Le variabili in nuBASIC possono essere dichiarate esplicitamente con `Dim` o create implicitamente
al primo assegnamento. Il **suffisso del tipo** aggiunto al nome della variabile ne determina il
tipo di memorizzazione. Lo stesso nome con suffissi diversi crea variabili distinte e non correlate.

| Suffisso | Tipo    | Intervallo / Note                     | Esempio            |
|--------|---------|---------------------------------------|--------------------|
| `%`    | Integer | 32 bit con segno, −2 147 483 648 .. +2 147 483 647 | `count% = 10` |
| `&`    | Long64  | Intero a 64 bit con segno             | `big& = 10000000000` |
| `#`    | Boolean | `True` o `False`                      | `flag# = True`     |
| `$`    | String  | Testo UTF-8 a lunghezza variabile     | `name$ = "nuBASIC"` |
| `@`    | Byte    | 0..255; usato negli array di byte     | `buf@(255)`        |
| *(nessuno)* | Double | Virgola mobile IEEE 754 a 64 bit  | `pi = 3.14159`     |

Il tipo `Any` viene dedotto automaticamente dal valore assegnato ed è utile nelle tabelle hash
e nelle funzioni generiche.

Dichiarazione esplicita con annotazione del tipo opzionale — obbligatoria prima di usare gli
array, e buona pratica per la chiarezza:

```basic
Dim counter As Integer
Dim name    As String
Dim ratio   As Double
Dim items(99) As Integer   ' array di 100 interi (indici 0..99)
Dim flag    As Boolean
```

Le costanti sono dichiarate con `Const` e non possono essere riassegnate dopo la dichiarazione.
Possono includere un tipo esplicito o lasciare che l'interprete lo deduca:

```basic
Const MAX_SIZE As Integer = 100
Const APP_NAME$ = "nuBASIC Demo"
Const PI_APPROX = 3.14159265
```

### 4.2 Operatori

#### Operatori aritmetici

nuBASIC fornisce gli operatori aritmetici standard. La divisione intera scarta la parte frazionaria
e può essere scritta come `\` o con la parola chiave `Div`. L'operatore modulo `Mod` restituisce
il resto. L'operatore `^` eleva un numero a una potenza.

```basic
a = 10 + 3     ' 13      — addizione
a = 10 - 3     ' 7       — sottrazione
a = 10 * 3     ' 30      — moltiplicazione
a = 10 / 3     ' 3.333…  — divisione in virgola mobile
a = 10 \ 3     ' 3       — divisione intera (tronca)
a = 10 Div 3   ' 3       — uguale alla barra rovesciata
a = 10 Mod 3   ' 1       — resto
a = 2 ^ 8      ' 256     — esponenziazione
```

I prefissi di incremento e decremento (stile C) modificano una variabile sul posto:

```basic
++counter%     ' equivalente a: counter% = counter% + 1
--counter%     ' equivalente a: counter% = counter% - 1
```

#### Operatori di confronto

Tutti gli operatori di confronto restituiscono un valore di verità numerico (diverso da zero per
vero, zero per falso) che può essere usato direttamente in `If`, `While` e altri contesti condizionali.

```basic
If a = b  Then ...   ' uguale a
If a <> b Then ...   ' diverso da
If a < b  Then ...   ' minore di
If a > b  Then ...   ' maggiore di
If a <= b Then ...   ' minore o uguale a
If a >= b Then ...   ' maggiore o uguale a
```

#### Operatori logici

`And`, `Or`, `Xor` operano su espressioni booleane. `Not` inverte un valore booleano. Questi
vengono valutati in corto circuito: `And` si ferma al primo operando falso, `Or` al primo vero.

```basic
If x > 0 And y > 0  Then Print "entrambi positivi"
If x = 0 Or  y = 0  Then Print "almeno uno zero"
If Not(flag#)        Then Print "flag è falso"
result# = (a > b) Xor (c > d)
```

#### Operatori bit a bit

Per la manipolazione a basso livello dei bit, nuBASIC fornisce un set completo di operatori bit a
bit. Questi operano sulla rappresentazione intera dei loro operandi.

```basic
result = a bAnd b    ' AND bit a bit  — bit impostati in entrambi
result = a bOr  b    ' OR bit a bit   — bit impostati in uno o nell'altro
result = a bXor b    ' XOR bit a bit  — bit impostati in uno ma non in entrambi
result = bNot(a)     ' NOT bit a bit  — tutti i bit invertiti
result = a bShl 2    ' scorrimento a sinistra di 2 posizioni  (moltiplica per 4)
result = a bShr 2    ' scorrimento a destra di 2 posizioni (divide per 4)
```

I letterali esadecimali si scrivono normalmente con il prefisso BASIC `&h` / `&H` e sono
particolarmente utili per i colori e le maschere di bit. nuBASIC accetta anche `0x` / `0X`
quando si copiano costanti da header C o documentazione di piattaforma:

```basic
mask%   = &hFF000000   ' solo il byte superiore
red%    = &h0000FF     ' GDI: canale blu nel byte basso
green%  = &h00FF00
blue%   = &hFF0000
white%  = &hFFFFFF
black%  = &h000000
same%   = 0xFFFFFF    ' sinonimo accettato di &hFFFFFF
```

### 4.3 Controllo del flusso

Il controllo del flusso determina in quale ordine vengono eseguite le istruzioni di un programma.
nuBASIC supporta sia lo stile BASIC classico (numeri di riga, `GoTo`, `GoSub`) sia la
programmazione completamente strutturata (`If/End If`, `For/Next`, `While/Wend`, `Sub`, `Function`).
I due stili possono essere liberamente mescolati.

#### If / ElIf / Else

L'istruzione `If` è il costrutto principale per le decisioni. La forma su singola riga è comoda
per brevi guardie; la forma su più righe (terminata da `End If`) consente più istruzioni in ogni
ramo e una catena illimitata di condizioni `ElIf`.

Forma su singola riga — sia il ramo `Then` sia il ramo opzionale `Else` stanno su una riga:

```basic
If score% > 100 Then Print "Punteggio massimo!" Else Print "Continua a provare"
```

Forma su più righe — ogni ramo può contenere un numero qualsiasi di istruzioni:

```basic
If score% >= 90 Then
   Print "Eccellente"
   grade$ = "A"
ElIf score% >= 75 Then
   Print "Buono"
   grade$ = "B"
ElIf score% >= 60 Then
   Print "Passato"
   grade$ = "C"
Else
   Print "Non passato"
   grade$ = "F"
End If
```

`ElseIf` è accettato come alias per `ElIf`. Un `If` all'interno di un altro `If` (annidamento)
è completamente supportato a qualsiasi profondità.

#### For / Next

Il ciclo `For` esegue un blocco di istruzioni un numero fisso di volte, avanzando una variabile
contatore da un valore iniziale a un valore finale. La clausola opzionale `Step` imposta
l'incremento; se omessa, l'incremento predefinito è 1. Un `Step` negativo conta all'indietro.

```basic
' Conta da 1 a 10
For i% = 1 To 10
   Print i%
Next i%

' Passo personalizzato: contatore in virgola mobile
For x = 0.0 To 1.0 Step 0.25
   Print x          ' stampa 0.0, 0.25, 0.5, 0.75, 1.0
Next x

' Conta all'indietro
For i% = 10 To 1 Step -1
   Print i%
Next i%

' Esci dal ciclo anticipatamente quando si verifica una condizione
For i% = 1 To 1000
   If i% Mod 7 = 0 And i% Mod 11 = 0 Then
      Print "Primo multiplo sia di 7 sia di 11: "; i%
      Exit For
   End If
Next i%
```

I cicli `For` possono essere annidati a qualsiasi profondità. Il nome della variabile dopo `Next`
è opzionale ma rende più facile leggere i cicli annidati.

#### While / Wend

Il ciclo `While` ripete il suo corpo finché la condizione è vera (diversa da zero). La condizione
viene verificata *prima* che il corpo venga eseguito, quindi se è falsa fin dall'inizio il corpo
non viene mai eseguito. `Wend` e `End While` sono intercambiabili come parola chiave di chiusura.

```basic
' Leggi le righe da un file fino a EOF
While Not(EOF(filenum%))
   Input# filenum%, line$
   Print line$
Wend

' Ciclo infinito con condizione di uscita esplicita
While 1
   key$ = InKey$()
   If key$ = "q" Or key$ = "Q" Then Exit While
   MDelay 10
Wend
```

#### Do / Loop While

Il ciclo `Do` è la controparte a post-condizione di `While`: esegue sempre il suo corpo almeno
una volta, poi verifica la condizione in fondo. Questa è la scelta naturale quando si deve
eseguire un'azione e poi decidere se ripeterla.

```basic
' Attendi la pressione di un tasto — il corpo viene eseguito almeno una volta
Do
   key$ = InKey$()
Loop While Len(key$) = 0

' Riprova fino a 10 volte
Do
   ++attempts%
   result% = TryOperation()
   If result% = 0 Then Exit Do    ' successo — esci anticipatamente
Loop While attempts% < 10

If result% <> 0 Then Print "Fallito dopo 10 tentativi"
```

#### GoTo / GoSub / Return

`GoTo` esegue un salto incondizionato a un numero di riga o a un'etichetta con nome. Le etichette
sono identificatori seguiti da due punti e possono apparire ovunque nel programma. Sebbene `GoTo`
sia spesso sconsigliato nel codice moderno — rende il flusso difficile da seguire — rimane
completamente supportato ed è talvolta il modo più chiaro per esprimere una macchina a stati o
un programma in stile classico.

```basic
GoTo gameLoop

init:
   score% = 0
   lives% = 3

gameLoop:
   ' corpo del ciclo principale
   If lives% = 0 Then GoTo gameOver
   GoTo gameLoop

gameOver:
   Print "Game over! Punteggio: "; score%
   End
```

`GoSub` salta a una subroutine identificata da un numero di riga o da un'etichetta, e `Return`
torna all'istruzione immediatamente successiva alla chiamata `GoSub`. A differenza di `Sub`
(vedi sotto), le subroutine `GoSub` condividono lo stesso ambito delle variabili con il chiamante
— non ci sono parametri o variabili locali. `GoSub` è il modo classico BASIC per condividere
un blocco di codice riutilizzabile senza il carico di una definizione di procedura completa.

```basic
GoSub DrawScreen
GoSub UpdateScore
GoTo mainLoop

DrawScreen:
   Cls
   FillRect 0, 0, 640, 480, &h000000
   Return

UpdateScore:
   TextOut 10, 10, "Score: " + Str$(score%), &hffffff
   Return
```

#### On / GoTo (salto calcolato)

`On expr GoTo label1, label2, …` seleziona un obiettivo di salto in base al valore intero di
un'espressione. Se l'espressione vale 0, il controllo va a `label1`; se vale 1, a `label2`;
e così via. Questo è il salto calcolato classico del BASIC, equivalente a uno switch/case senza
fall-through.

```basic
' Seleziona la schermata di difficoltà
On game_mode% GoTo beginner, intermediate, expert

beginner:
   mines%  = 10 : Print "Principiante (10 mine)"  : GoTo setup_done
intermediate:
   mines%  = 20 : Print "Intermedio (20 mine)" : GoTo setup_done
expert:
   mines%  = 40 : Print "Esperto (40 mine)"
setup_done:
```

#### Select Case

`Select Case` / `End Select` smista il controllo in base a un'espressione scalare. I rami
`Case` vengono valutati in ordine; il primo che corrisponde viene eseguito e il controllo
passa a `End Select`. `Case Else` è il ramo facoltativo di default.

```basic
Select Case punteggio%
   Case Is >= 90
      voto$ = "A"
   Case Is >= 75
      voto$ = "B"
   Case Is >= 60
      voto$ = "C"
   Case Else
      voto$ = "F"
End Select
```

Forme di ramo supportate:

| Forma | Esempio | Corrisponde quando… |
| --- | --- | --- |
| Valore singolo | `Case 1` | espressione = 1 |
| Lista di valori | `Case 1, 3, 5` | espressione è uno dei valori elencati |
| Intervallo | `Case 1 To 10` | 1 ≤ espressione ≤ 10 |
| Confronto | `Case Is > 100` | espressione soddisfa il confronto |
| Default | `Case Else` | nessun ramo precedente ha corrisposto |

Funziona con interi, double e stringhe. I blocchi `Select Case` possono essere annidati.

```basic
' Smistamento su stringa
Select Case cmd$
   Case "esci", "quit", "q"
      Print "Arrivederci" : End
   Case "aiuto", "?"
      Print "Digita un comando"
   Case Else
      Print "Sconosciuto: "; cmd$
End Select
```

### 4.4 Subroutine e funzioni

nuBASIC supporta due tipi di procedure con nome: `Sub` (senza valore di ritorno) e `Function`
(restituisce un valore). Entrambi accettano parametri tipizzati, creano il proprio ambito di
variabili locali e possono chiamarsi ricorsivamente. A differenza delle subroutine `GoSub`,
`Sub` e `Function` non condividono le variabili con il chiamante — ogni invocazione ha il
proprio set privato di variabili locali.

#### Sub — una procedura senza valore di ritorno

Un `Sub` raggruppa un insieme di istruzioni correlate sotto un nome. Riceve i propri input come
parametri e opera su di essi o su variabili globali. `Exit Sub` fornisce un percorso di uscita anticipata.

```basic
Sub ClearArea(x1%, y1%, x2%, y2%)
   FillRect x1%, y1%, x2%, y2%, &h000000
   Rect     x1%, y1%, x2%, y2%, &h404040
End Sub

Sub PrintCentered(msg$, row%, color%)
   col% = (80 - Len(msg$)) \ 2
   Locate row%, col%
   Print msg$
End Sub

' Chiamate:
ClearArea 0, 0, 640, 480
PrintCentered "Benvenuto in nuBASIC", 12, &hffffff
```

#### Function — una procedura che restituisce un valore

Una `Function` calcola e restituisce un singolo valore. Il tipo di ritorno è indicato dal suffisso
sul nome della funzione o da una clausola esplicita `As Type`. Il valore di ritorno viene impostato
assegnando al nome della funzione stessa all'interno del corpo.

```basic
Function Factorial%(n%)
   If n% <= 1 Then
      Factorial% = 1
   Else
      Factorial% = n% * Factorial%(n% - 1)   ' chiamata ricorsiva
   End If
End Function

Function Clamp(value, minVal, maxVal)
   If value < minVal Then
      Clamp = minVal
   ElIf value > maxVal Then
      Clamp = maxVal
   Else
      Clamp = value
   End If
End Function

Function Greeting$(name$)
   Greeting$ = "Ciao, " + name$ + "! Benvenuto in nuBASIC."
End Function

' Chiamate:
Print Factorial%(12)
Print Clamp(1.5, 0.0, 1.0)     ' 1.0
Print Greeting$("Mondo")
```

Le funzioni possono restituire stringhe, double, interi o qualsiasi altro tipo. Possono anche
restituire array per valore. `Exit Function` esce anticipatamente, lasciando come valore di
ritorno l'ultimo valore assegnato al nome della funzione.

#### Qualificatori di parametro ByRef e ByVal

Per impostazione predefinita ogni parametro viene passato **per valore** (`ByVal`): il Sub o
la Function riceve una copia del valore del chiamante, quindi le modifiche interne alla
procedura non hanno effetto sul chiamante. Anteporre `ByRef` a un parametro per passarlo
**per riferimento**: il chiamato opera direttamente sulla variabile del chiamante, e qualsiasi
assegnazione è visibile al chiamante dopo il ritorno dalla chiamata.

Sia i valori scalari sia le variabili `Struct` possono essere passati con `ByRef`:

```basic
Sub Scambia(ByRef a% As Integer, ByRef b% As Integer)
   Dim tmp% As Integer
   tmp% = a%
   a% = b%
   b% = tmp%
End Sub

Dim x% As Integer, y% As Integer
x% = 7 : y% = 42
Call Scambia(x%, y%)
Print x%, y%   ' stampa: 42   7

' -------------------------------------------------------
Struct Punto
   x As Double
   y As Double
End Struct

Sub Trasla(ByRef p As Punto, dx As Double, dy As Double)
   p.x = p.x + dx
   p.y = p.y + dy
End Sub

Dim pt As Punto
pt.x = 10.0 : pt.y = 20.0
Call Trasla(pt, 3.0, -5.0)
Print pt.x, pt.y   ' stampa: 13   15
```

Quando `ByVal` è esplicito, il nome del parametro chiarisce l'intento nel codice sorgente:

```basic
Sub MostraIlDoppio(ByVal n As Integer)
   n = n * 2
   Print n         ' stampa il valore raddoppiato
End Sub

Dim v% As Integer
v% = 5
Call MostraIlDoppio(v%)
Print v%            ' ancora 5 — la copia del chiamante non è modificata
```

#### Parola chiave Call e chiamate Sub con parentesi

Una Sub puo' essere invocata in tre forme equivalenti:

```basic
ClearArea 0, 0, 640, 480            ' BASIC classico: chiamata bare, senza parentesi
ClearArea(0, 0, 640, 480)           ' chiamata con parentesi
Call ClearArea(0, 0, 640, 480)      ' parola chiave Call esplicita (parentesi obbligatorie)
```

Le tre forme producono la stessa chiamata. La forma con parentesi
(`NomeSub(argomenti)`) e' stata introdotta per allinearsi alle chiamate di
Function e per accomodare lo stile dei dialetti BASIC moderni. Il parser
distingue `array(i) = valore` (assegnazione di array) da una chiamata Sub
controllando la presenza di un `=` al livello principale della riga.

Per le Sub senza argomenti sono accettate sia `Saluta` sia `Saluta()`:

```basic
Sub Saluta()
   Print "Ciao!"
End Sub

Saluta
Saluta()
Call Saluta()
```

`Call` e' una parola chiave opzionale. Quando viene usata, la lista degli
argomenti **deve** essere racchiusa tra parentesi. La forma con `Call`
e' familiare ai programmatori provenienti da Visual Basic o da altri
dialetti BASIC.

#### Direttiva Include

La direttiva `Include` (o `#Include`) carica ed esegue un altro file sorgente nel punto in
cui appare la direttiva. Ciò rende semplice suddividere un programma in più file o
condividere routine di libreria comuni:

```basic
' principale.bas
Include "utilità.bas"
Include "grafica.bas"

Call DisegnaCornice(0, 0, 639, 479)
```

```basic
' utilità.bas
Sub DisegnaCornice(x1%, y1%, x2%, y2%)
   Rect x1%, y1%, x2%, y2%, &hffffff
End Sub
```

Il percorso del file viene risolto rispetto alla directory contenente il file che emette la
direttiva. `Include` viene elaborato al momento del caricamento, quindi tutte le definizioni
nel file incluso sono disponibili al resto del file che lo include.

#### Progetti multi-sorgente (`.nbp`)

Un file di progetto nuBASIC (`.nbp`) consente all'IDE e al debugger di
raggruppare uno script entry-point con i file che include. Il file di
progetto e' un piccolo manifest testuale chiave=valore:

```text
name = MioProgramma
syntax = modern
entry = src/main.bas
```

| Chiave | Significato |
|--------|-------------|
| `name` | Nome visualizzato dall'IDE nella barra del titolo e nei progetti recenti |
| `syntax` | `legacy` o `modern` — modalita' `Syntax` di default per il file entry |
| `entry` | Percorso del file `.bas` di entry, risolto rispetto alla directory contenente il `.nbp` |

Apri un `.nbp` dall'IDE Windows con **File → Apri progetto** o passa il
suo percorso da riga di comando. I file inclusi vengono comunque risolti
dalle direttive `Include` all'interno dell'entry-point — il `.nbp`
registra solo *quale* file e' l'entry, la modalita' di sintassi e il
nome del progetto. L'estensione VS Code legge lo stesso formato per
avviare il debugger.

#### Parametri array a dimensione aperta

Una Sub o Function può accettare un array di qualsiasi dimensione dichiarando il parametro
con parentesi vuote — `param() As Tipo`. Questo elimina la necessità di fissare la
dimensione nella firma e permette di scrivere procedure generiche:

```basic
Sub SommaArray(numeri() As Integer, ByRef totale% As Integer)
   totale% = 0
   Dim i% As Integer
   For i% = 0 To SizeOf(numeri) - 1
      totale% = totale% + numeri(i%)
   Next i%
End Sub

Dim dati%(4)
dati%(0) = 10 : dati%(1) = 20 : dati%(2) = 30 : dati%(3) = 40 : dati%(4) = 50
Dim s% As Integer
Call SommaArray(dati%(), s%)
Print s%   ' 150
```

Il chiamante passa l'array con il suffisso `()`; il callee accede agli elementi con la
normale notazione a indice.

---

### 4.5 Classi e oggetti

nuBASIC supporta la programmazione orientata agli oggetti come stile di prima classe
accanto al BASIC classico e a quello strutturato. L'implementazione copre le funzionalita'
attese da un sistema OOP piccolo ma completo:

- **Incapsulamento** — campi e metodi hanno una visibilita' `Public` / `Protected` /
  `Private`, verificata in fase di compilazione. `Public` e' il default; `Private` e'
  raggiungibile solo all'interno della classe dichiarante; `Protected` estende la
  visibilita' alle classi derivate (la catena di ereditarieta').
- **Ereditarieta' singola** — `Inherits` dichiara la classe base. Campi e membri `Public`
  / `Protected` sono raggiungibili dalla classe derivata.
- **Polimorfismo** — i metodi marcati `Overridable` possono essere sostituiti da metodi
  `Overrides` nelle classi derivate; l'override deve corrispondere esattamente alla
  signature della base (Sub vs Function, tipo di ritorno, numero e tipi dei parametri,
  `ByRef` / `ByVal`). Le chiamate attraverso un riferimento alla base vengono dispacciate
  dinamicamente all'implementazione piu' derivata.
- **Dispatch esplicito alla base** — dentro un override, `MyBase.Membro(...)` chiama la
  versione del membro della classe base immediata, evitando il dispatch virtuale.
- **Semantica di riferimento** — le variabili tipate per classe contengono riferimenti;
  assegnare un oggetto a un altro li lega alla stessa istanza, in contrasto con la
  semantica per copia di valore di `Struct`.
- **Metodi statici** — `Static Function` / `Static Sub` sono procedure a livello di classe
  invocabili come `NomeClasse.Metodo(...)`, senza un'istanza implicita.
- **Costruzione e distruzione** — `Sub New(...)` e' il costruttore (richiamato da `Dim
  x As New T(...)`); `Sub Delete()` e' il distruttore e viene chiamato automaticamente
  quando un'istanza locale "posseduta" esce dallo scope (RAII), in ordine inverso di
  dichiarazione, e nelle classi derivate il distruttore derivato precede quello base.

Il resto di questa sezione esamina queste funzionalita' nell'ordine. La
[panoramica dei paradigmi in §2](#2-introduzione-a-nubasic) spiega come l'OOP si inserisce
accanto agli stili classico e strutturato nello stesso file sorgente.

Le classi raggruppano dati correlati (campi) e comportamenti (metodi) sotto un unico tipo.
Si dichiarano con `Class`/`End Class`; le istanze si creano con `Dim`.

```basic
Class Rettangolo
   Public larghezza  As Double
   Public altezza    As Double

   Function Area() As Double
      Area = Me.larghezza * Me.altezza
   End Function

   Function Perimetro() As Double
      Perimetro = 2.0 * (Me.larghezza + Me.altezza)
   End Function
End Class

Dim r As Rettangolo
r.larghezza = 8.0
r.altezza   = 5.0
Print "Area ="; r.Area()         ' 40
Print "Perimetro ="; r.Perimetro()  ' 26
```

`Me` è un riferimento implicito all'istanza corrente all'interno di ogni metodo di istanza.

#### Metodi statici

`Static Function` e `Static Sub` all'interno di un blocco `Class` definiscono procedure
a livello di classe. Non ricevono un'istanza implicita e non possono usare `Me`. Si
invocano con `NomeClasse.Metodo(argomenti)` — senza bisogno di un'istanza:

```basic
Class Util
   Static Function Massimo(a As Integer, b As Integer) As Integer
      If a > b Then Massimo = a Else Massimo = b
   End Function

   Static Sub StampaProdotto(a As Integer, b As Integer)
      Print "Prodotto ="; a * b
   End Sub
End Class

Dim risultato% As Integer
risultato% = Util.Massimo(3, 7)    ' 7
Util.StampaProdotto 4, 5            ' Prodotto = 20
```

Metodi statici e di istanza possono coesistere nella stessa classe. I metodi statici sono
ideali per funzioni di utilità e factory che non necessitano di stato dell'istanza.

#### Distruttori

Una classe puo' definire `Sub Delete()` come distruttore. Per le istanze
locali "possedute", nuBASIC chiama il distruttore automaticamente quando
lo scope della procedura termina. Gli oggetti locali vengono distrutti in
ordine inverso di definizione e una classe derivata esegue il proprio
distruttore prima di quello della classe base.

```basic
Class Risorsa
   Public nome$ As String

   Sub New(n$ As String)
      Me.nome$ = n$
   End Sub

   Sub Delete()
      Print "chiusura "; Me.nome$
   End Sub
End Class

Sub Demo()
   Dim primo As New Risorsa("primo")
   Dim secondo As New Risorsa("secondo")
End Sub

Demo()
' stampa:
' chiusura secondo
' chiusura primo
```

Gli oggetti passati come parametri presi in prestito non vengono distrutti
dal chiamato.

#### Modificatori di accesso: Public, Protected, Private

I membri possono essere etichettati con uno dei tre livelli di visibilita':

| Modificatore | Dove il membro e' raggiungibile |
|--------------|---------------------------------|
| `Public` | Ovunque (default se omesso) |
| `Protected` | Dentro la classe che lo dichiara **e dentro qualunque classe derivata** |
| `Private` | Solo dentro la classe che lo dichiara |

```basic
Class Animale
   Public  nome$    As String
   Protected energia% As Integer    ' visibile alle sottoclassi
   Private  segreto$  As String     ' nascosto alle sottoclassi

   Sub New(n$ As String)
      Me.nome$    = n$
      Me.energia% = 100
      Me.segreto$ = "interno ad Animale"
   End Sub
End Class

Class Cane
   Inherits Animale

   Sub Abbaia()
      ' OK: i campi Protected sono visibili lungo la catena di ereditarieta'.
      Me.energia% = Me.energia% - 5
      Print Me.nome$; " abbaia (energia="; Me.energia%; ")"
      ' Errore di compilazione se decommentata — Private non si eredita:
      ' Print Me.segreto$
   End Sub
End Class
```

`Public` e' il default se non si specifica nulla. La visibilita' viene
verificata in fase di compilazione: un accesso non autorizzato genera una
diagnostica precisa che identifica il membro coinvolto.

#### Ereditarieta' e override: MyBase

Si usa `Inherits NomeBase` per derivare da una classe, `Overridable` per
marcare un metodo che le sottoclassi possono ridefinire e `Overrides`
sulla ridefinizione. All'interno di un metodo che effettua override,
`MyBase.Membro` invia la chiamata **esplicitamente** al membro
corrispondente della classe base immediata, evitando il dispatch virtuale
che altrimenti rientrerebbe nell'override stesso.

```basic
Class Forma
   Overridable Function Descrivi$() As String
      Descrivi$ = "una forma"
   End Function
End Class

Class Cerchio
   Inherits Forma

   Public raggio As Double

   Overrides Function Descrivi$() As String
      ' Riusa l'implementazione base e la decora.
      Descrivi$ = MyBase.Descrivi$() + " (un cerchio di raggio " + Str$(Me.raggio) + ")"
   End Function
End Class

Dim c As New Cerchio
c.raggio = 2.5
Print c.Descrivi$()   ' una forma (un cerchio di raggio 2.5)
```

Regole verificate in fase di compilazione:

- `Overrides` viene rifiutato se nella catena base non esiste un metodo
  `Overridable` con la **stessa** signature (Sub vs Function, tipo di
  ritorno, numero e tipi dei parametri, `ByRef` / `ByVal`).
- `MyBase.Membro(...)` e' consentito solo all'interno di un metodo di
  istanza e il membro deve esistere nella catena base ed essere visibile
  al chiamante (`Public` o `Protected`).
- Le catene a piu' livelli funzionano come ci si aspetta: `MyBase` punta
  sempre alla base **immediata** della classe lessicale, anche se a
  runtime l'oggetto e' di un tipo derivato piu' in basso nella gerarchia.

---

### 4.6 Punto di ingresso main()

Se il programma di primo livello definisce una `Function` denominata `main`, l'interprete
la chiama come punto di ingresso anziché eseguire le istruzioni dall'inizio del file. La
funzione deve restituire un `Integer` (codice di uscita del processo). Sono supportate tre
firme:

```basic
' Forma più semplice — nessun argomento
Function main() As Integer
   Print "Ciao da main"
   main = 0
End Function
```

```basic
' Solo conteggio argomenti
Function main(argc As Integer) As Integer
   Print "Numero di argomenti:"; argc
   main = 0
End Function
```

```basic
' argc e argv completi
Function main(argc As Integer, argv() As String) As Integer
   Dim i% As Integer
   Print "Script: "; argv(0)
   For i% = 1 To argc - 1
      Print "  Arg "; i%; ": "; argv(i%)
   Next i%
   main = 0   ' zero = successo
End Function
```

`argc` è uguale a 1 più il numero di argomenti extra passati dalla riga di comando.
`argv(0)` è il nome del file script; `argv(1)` fino a `argv(argc-1)` sono gli argomenti
aggiuntivi forniti dopo il nome del file:

```sh
nubasic -t -e mioprogramma.bas ciao mondo
' → argc = 3
' → argv(0) = "mioprogramma.bas"
' → argv(1) = "ciao"
' → argv(2) = "mondo"
```

Quando `main` è definita in un file `Include`to, il comportamento di punto di ingresso
viene soppresso in quel file — viene invocata solo la `main` del programma di primo livello.

---

### 4.7 Moduli con namespace

In **modalità sintassi moderna** (`Syntax Modern`), la libreria integrata è organizzata in
moduli con nome. Le funzioni possono essere chiamate con il prefisso qualificato
`modulo::nome`. `Syntax Legacy` (la modalità predefinita) ripristina il namespace
piatto classico, quindi tutti i programmi esistenti continuano a funzionare invariati.

```basic
Syntax Modern

' Chiamate qualificate
Dim s As Double
s = math::sin(0.785398)          ' ≈ 0.7071
Dim h$ As String
h$ = string::left$("Ciao", 3)    ' "Cia"
Dim n% As Integer
n% = runtime::sizeof(mioArray)
```

#### Using — Importare un modulo

`Using NomeModulo` importa tutti i nomi di un modulo nello scope corrente, eliminando la
necessità del prefisso `modulo::`:

```basic
Syntax Modern
Using math
Using string

Dim v As Double
v = Sin(0.785398)         ' math::sin senza prefisso
Dim s$ As String
s$ = Left$("Mondo", 3)   ' string::left$ senza prefisso
```

#### Cambio di modalità

`Syntax Legacy` può comparire in qualsiasi punto; ogni file `Include`to rispetta la
modalità attiva nel punto di inclusione.

```basic
Syntax Legacy
' I nomi globali classici tornano a funzionare
Print Len("abc")    ' 3
Print Sin(0)        ' 0
```

---

### 4.8 Strutture

Un `Struct` definisce un tipo di dati composito che raggruppa diversi campi con nome sotto un
unico nome. I campi possono essere di qualsiasi tipo integrato, e le strutture possono essere
annidate — il campo di una struttura può essere di un altro tipo struttura. Una volta definita,
una struttura viene istanziata con `Dim`, e i suoi campi sono accessibili con la notazione a punto.

Le strutture sono particolarmente utili per organizzare dati correlati che appartengono insieme:
coordinate, riquadri delimitatori, entità di gioco, record di configurazione.

```basic
Struct Vector2D
   x As Double
   y As Double
End Struct

Struct Sprite
   pos    As Vector2D
   width% As Integer
   height% As Integer
   name$  As String
End Struct

Dim hero As Sprite
hero.pos.x   = 100.0
hero.pos.y   = 200.0
hero.width%  = 32
hero.height% = 48
hero.name$   = "Hero"

Print "Sprite "; hero.name$; " in ("; hero.pos.x; ", "; hero.pos.y; ")"
```

Gli array di strutture sono dichiarati nel modo normale:

```basic
Dim enemies(10) As Sprite
enemies(0).name$ = "Goblin"
enemies(0).pos.x = 50
```

### 4.9 Array

Gli array memorizzano più valori dello stesso tipo sotto un unico nome, accessibili tramite un
indice numerico. In nuBASIC, gli array devono essere dichiarati con `Dim` prima dell'uso, e la
dichiarazione specifica l'indice massimo (l'array avrà `n+1` elementi, indicizzati da `0` a `n`).

```basic
Dim scores%(9)      ' 10 interi, indici 0..9
Dim names$(4)       ' 5 stringhe, indici 0..4
Dim data(99)        ' 100 double, indici 0..99
```

Gli elementi sono accessibili e assegnabili usando le parentesi:

```basic
scores%(0) = 95
scores%(1) = 87
scores%(2) = 72

' Itera con For
total% = 0
For i% = 0 To 9
   total% = total% + scores%(i%)
Next i%
Print "Media: "; total% / 10
```

`ReDim` ridimensiona un array esistente. Tutto il contenuto corrente viene scartato e gli elementi
vengono reimpostati a zero o a stringa vuota. Usatelo quando la dimensione richiesta non è nota
all'avvio del programma:

```basic
size% = 100
ReDim scores%(size% - 1)   ' ora contiene size% elementi
```

Gli array di byte (suffisso `@`) sono la scelta efficiente per i dati binari — buffer di file,
manipolazione di byte grezzi, dati pixel delle immagini:

```basic
Dim buf@(1023)        ' buffer da 1024 byte
buf@(0) = 65          ' ASCII 'A'
buf@(1) = 66          ' ASCII 'B'
Print Chr$(buf@(0)) + Chr$(buf@(1))   ' AB
```

I dati multidimensionali vengono gestiti dichiarando un array 1-D e calcolando l'indice
manualmente con la formula standard row-major `row * width + col`:

```basic
Const W% = 10
Dim grid%(W% * W% - 1)   ' griglia logica 10×10

' Accedi all'elemento in (row=2, col=3):
grid%(2 * W% + 3) = 42
Print grid%(2 * W% + 3)
```

### 4.10 Tabelle hash

Le tabelle hash (dette anche dizionari o array associativi) mappano chiavi stringa a valori di
qualsiasi tipo. In nuBASIC una tabella hash è identificata da una stringa di nome piuttosto che
da una variabile — il runtime mantiene un registro globale di tabelle con nome. Qualsiasi
combinazione di nome e chiave può essere memorizzata e recuperata in qualsiasi momento.

Questo design rende le tabelle hash particolarmente utili per archivi di configurazione, tabelle
di ricerca, cache e qualsiasi scenario in cui è necessario associare un valore a un'etichetta
descrittiva piuttosto che a un indice numerico.

```basic
' Inserisci o aggiorna voci
HSet "config", "width",     800
HSet "config", "height",    600
HSet "config", "title$",    "Il mio gioco"
HSet "config", "fullscreen#", False

' Recupera i valori
w% = HGet%("config", "width")
h% = HGet%("config", "height")
t$ = HGet$("config", "title$")
Print t$; " — "; w%; "x"; h%

' Controlla se una chiave esiste prima di leggere
If HChk("config", "fullscreen#") Then
   fs# = HGet#("config", "fullscreen#")
   If fs# Then Print "Modalità schermo intero"
End If

' Conta le voci in una tabella
Print "Voci di config: "; HCnt("config")

' Elimina una singola chiave
HDel "config", "fullscreen#"

' Elimina l'intera tabella (libera tutta la memoria)
HDel "config"
```

### 4.11 I/O su file

nuBASIC fornisce tre livelli di accesso ai file: **sequenziale** (testo orientato alle righe),
**binario** (accesso a livello di byte tramite `FOpen` e `Read#`), e **ad accesso casuale**
(ricercabile). I file sono referenziati da un numero intero di file (`#1`, `#2`, …) che si
assegna all'apertura.

#### File di testo sequenziali

L'I/O sequenziale è il modello più semplice: si apre il file per input o output, si leggono o
scrivono righe e lo si chiude quando si è finito. Il puntatore del file avanza automaticamente
con ogni chiamata a `Input#` o `Print#`.

```basic
' Scrivi un file di testo
Open "notes.txt" For Output As #1
Print# 1, "Prima riga"
Print# 1, "Seconda riga"
Print# 1, "Punteggio: " + Str$(score%)
Close #1

' Rileggi riga per riga
Open "notes.txt" For Input As #2
While Not(EOF(2))
   Input# 2, line$
   Print line$
Wend
Close #2

' Aggiungi senza sovrascrivere
Open "notes.txt" For Append As #1
Print# 1, "Nuova voce al " + SysTime$()
Close #1
```

#### Binario e accesso casuale

Per i file binari, `FOpen` accetta stringhe di modalità in stile C (`"r"`, `"w"`, `"a"`, `"r+"`, …),
dando un controllo preciso sul comportamento di lettura/scrittura/creazione. `Read#` legge un
numero fisso di byte direttamente in una variabile, e `Seek` riposiziona il puntatore del file.

```basic
' Scrivi byte grezzi
FOpen "data.bin", "wb", 1
Dim buf@(3)
buf@(0) = &hDE : buf@(1) = &hAD : buf@(2) = &hBE : buf@(3) = &hEF
Print# 1, buf@
Close #1

' Posizionati su un offset noto e leggi
FOpen "data.bin", "rb", 2
Seek 2, 2, 2           ' posizione 2 byte dall'inizio (origin=2)
Read# 2, b@, 1         ' leggi 1 byte in b@
Print "Byte all'offset 2: "; Hex$(b@)
Close #2
```

Funzioni diagnostiche utili — verificatele dopo qualsiasi operazione su file nel codice di produzione:

```basic
pos%  = FTell(1)    ' offset byte corrente
size% = FSize(1)    ' dimensione totale del file in byte
err%  = FError(1)   ' diverso da zero se si è verificato un errore su questo file
at_end% = EOF(1)    ' 1 se il puntatore del file è alla fine
code% = Errno()     ' codice di errore di sistema dall'ultima operazione fallita
Print Errno$(code%) ' descrizione dell'errore leggibile dall'uomo
```

#### Operazioni sul file system

```basic
MkDir("saves")            ' crea una directory
RmDir("saves/old")        ' rimuove una directory vuota
Erase("temp.txt")         ' elimina un file
Print Pwd$()              ' stampa la directory di lavoro corrente
ChDir ".."                ' cambia alla directory genitore
```

### 4.12 DATA, READ, RESTORE

`Data`, `Read` e `Restore` implementano un meccanismo classico del BASIC per incorporare tabelle
di costanti strutturate direttamente nel codice sorgente. Gli elementi di dati vengono memorizzati
in un archivio sequenziale quando vengono eseguite le istruzioni `Data`; `Read` li recupera in
ordine; `Restore` azzera o riposiziona il puntatore di lettura.

Questo è particolarmente adatto per tabelle di ricerca, definizioni di sprite, mappe di livello,
note musicali e qualsiasi altro set di dati fisso che altrimenti richiederebbe un file esterno o
una lunga serie di istruzioni di assegnazione.

```basic
' Incorpora una tavolozza di colori
Data "rosso",    &hFF0000
Data "verde",    &h00FF00
Data "blu",      &h0000FF
Data "giallo",   &hFFFF00
Data "bianco",   &hFFFFFF

' Leggi e usa la tavolozza
For i% = 0 To 4
   Read name$, color%
   FillRect i%*60, 10, i%*60+50, 60, color%
   TextOut  i%*60+5, 70, name$, &hFFFFFF
Next i%

' Riavvolgi e rileggi dall'inizio
Restore
Read first_name$, first_color%
Print "Primo colore: "; first_name$

' Salta a una posizione specifica nell'archivio
Restore 2        ' salta al terzo elemento (indice 2, base zero)
Read name$, color%
Print "Terzo colore: "; name$

' Cancella l'intero archivio dati
Restore -1
```

Un pattern comune è usare `Data` per guidare una macchina a stati o per inizializzare gli array
all'avvio, evitando assegnazioni di indici hard-coded sparse nel codice.

### 4.13 Gestione delle stringhe

Le stringhe in nuBASIC sono sequenze di caratteri a lunghezza variabile. Tutti i letterali stringa
nel codice sorgente sono memorizzati come UTF-8, e la console GDI li renderizza correttamente per
qualsiasi script supportato dal font installato. Le funzioni stringa standard operano sulle posizioni
dei caratteri; la maggior parte usa l'indicizzazione a base 1 (seguendo la tradizione BASIC), ma
`SubStr$` e `PStr$` sono a base 0 (per coerenza con il modello di array di byte sottostante).

#### Misurazione e suddivisione

```basic
s$ = "Hello, World!"

Print Len(s$)           ' 13 — numero di caratteri
Print Left$(s$, 5)      ' "Hello"
Print Right$(s$, 6)     ' "World!"
Print Mid$(s$, 8, 5)    ' "World"   (base 1: inizia dalla posizione 8)
Print SubStr$(s$, 7, 5) ' "World"   (base 0: inizia dall'offset 7)
```

#### Ricerca

`InStr` non è sensibile alle maiuscole; `InStrCS` è sensibile alle maiuscole. Entrambe restituiscono
una posizione a base 1, o −1 se la sottostringa non viene trovata.

```basic
Print InStr("Hello World", "world")     ' 7  (trovato, non sensibile alle maiuscole)
Print InStr("Hello World", "xyz")       ' -1 (non trovato)
Print InStrCS("Hello World", "World")   ' 7  (corrispondenza sensibile alle maiuscole)
Print InStrCS("Hello World", "world")   ' -1 (sensibile alle maiuscole, nessuna corrispondenza)
```

#### Modifica e costruzione

`PStr$` sostituisce un singolo carattere a una posizione a base 0, restituendo la stringa modificata.
Questo è utile per costruire o correggere buffer di stringhe a larghezza fissa (come usato
internamente in `minesHunter.bas` e programmi simili):

```basic
board$ = ".........."        ' riga del tabellone da 10 caratteri
board$ = PStr$(board$, 3, "*")   ' "..*......."  — posiziona una mina alla colonna 3
board$ = PStr$(board$, 7, "F")   ' "..*.....F."  — bandiera alla colonna 7
```

#### Conversione maiuscole/minuscole e dei caratteri

```basic
Print UCase$("hello")    ' "HELLO"
Print LCase$("WORLD")    ' "world"
Print Asc("A")           ' 65  — punto di codice ASCII/Unicode
Print Chr$(65)           ' "A" — carattere dal punto di codice
Print Spc(4)             ' "    " — quattro spazi
```

#### Conversioni numeriche

```basic
n% = Val%("42")           ' stringa a intero
x  = Val("3.14")          ' stringa a double
Print Str$(3.14)          ' "3.14"
Print Hex$(255)           ' "FF"
Print StrP$(3.14159, 4)   ' "3.142" — arrotondato a 4 cifre significative
```

#### Sequenze di escape e output Unicode

Le sequenze di escape standard funzionano all'interno delle stringhe tra virgolette doppie:

```basic
Print "Colonna1\tColonna2\tColonna3"    ' separato da tabulazioni
Print "Riga 1\nRiga 2"                  ' newline incorporato
Print "Avviso!\a"                        ' carattere di avviso acustico
```

Per i caratteri Unicode oltre ASCII, usate il prefisso `$u` con le sequenze `\uXXXX`:

```basic
Print $u, "Caf\u00e9 — \u00e0 \u00e8 \u00ec"   ' Café — à è ì
Print $u, "\u03b1 \u03b2 \u03b3"               ' α β γ  (greco)
Print $u, "\u4e2d\u6587"                        ' 中文   (cinese)
```

#### Valutazione dinamica

`Eval` analizza e valuta una stringa come espressione nuBASIC in fase di esecuzione, con accesso
a tutte le variabili attualmente definite. Questo consente di costruire espressioni in modo
programmatico:

```basic
x = 5
expr$ = "x * x + 2 * x + 1"
Print Eval(expr$)       ' 36 — valutato con x=5
```

---
### 4.14 Chiamate a librerie native

Su Windows, Linux e macOS nuBASIC puo' dichiarare e chiamare funzioni
esportate da librerie native condivise. Il caricamento usa `LoadLibraryW`
su Windows e `dlopen(RTLD_LAZY|RTLD_LOCAL)` su Linux/macOS; l'invocazione
e' supportata da libffi su tutte le piattaforme.

```basic
Syntax Modern

' Windows
Declare Function GetCurrentProcessId Lib "kernel32.dll" () As DWORD
Print GetCurrentProcessId()
```

```basic
Syntax Modern

' Linux (glibc)
Declare Function strlen Lib "libc.so.6" (text As String) As Integer
Print strlen("hello")    ' stampa 5
```

```basic
Syntax Modern

' macOS
Declare Function strlen Lib "libSystem.B.dylib" (text As String) As Integer
Print strlen("hello")    ' stampa 5
```

La forma generale e':

```basic
Declare Function nome Lib "library" _
    [Alias "export"] _
    [CallConv "default" | "cdecl" | "stdcall"] _
    (param As NativeType, ...) As NativeType
```

Il valore di `Lib` viene passato cosi' com'e' al loader della piattaforma:
usare i soname canonici dell'host (`kernel32.dll`, `libc.so.6`,
`libSystem.B.dylib`) oppure un percorso assoluto. La calling convention
viene letta e memorizzata, ma su tutte le piattaforme supportate viene
usata l'ABI di default (x64 SysV / Windows x64).

I tipi nativi supportati sono `Integer`, `DWORD`, `Long64`, `ULong64`,
`Double`, `Bool`, `Pointer`, `String` e `Void`. `String` e' un puntatore
narrow `const char*`; per buffer modificabili e strutture costruite
manualmente si usa `Pointer`.

Gli helper di memoria nativa sono nel modulo `runtime` e si comportano
allo stesso modo su tutte le piattaforme:

```basic
Syntax Modern
Using runtime

p = NativeAlloc(32)
NativePokeStr p, 0, "hello", 32
Print NativePeekStr$(p, 0, 32)
NativeFree p
```

Le chiamate native sono abilitate per default negli host locali fidati e
si possono disabilitare con `--disable-native-calls`. Vedi
`docs/native-dll-calls.md`, `examples/native/native_dll_function.bas` ed
`examples/native/native_open_file_dialog.bas` per esempi Win32; su POSIX lo
stesso meccanismo raggiunge
`libc`, `libm`, `libpthread` e qualsiasi `.so` o `.dylib` costruito
dall'utente.

---

### 4.15 Continuazione di riga

Un underscore alla fine di una riga fisica unisce la riga successiva nello stesso
statement BASIC logico:

```basic
Declare Function lstrlenA Lib "kernel32.dll" _
    Alias "lstrlenA" _
    (text As String) As Integer

text$ = "hello " _
    + "world"
```

Il marker `_` deve essere l'ultimo carattere di codice non blank; puo essere
seguito da un commento con apostrofo. Il debugger considera la sequenza come un
solo statement e si ferma sulla prima riga fisica.

---

## 5. Grafica e Multimedia

Tutte le funzioni grafiche sono disponibili nella versione completa (Windows GDI o Linux/X11). Sono
assenti nella *build console*, orientata agli ambienti minimali.

Il sistema di coordinate ha l'origine nell'angolo **in alto a sinistra** dell'area client della finestra
di lavoro, con X che cresce verso destra e Y che cresce verso il basso. Tutte le coordinate sono in pixel.

I colori sono interi RGB compatti. Il modo più leggibile per specificarli è la notazione esadecimale con il
prefisso `&h`. **Importante**: i valori colore di nuBASIC seguono l'ordine dei byte GDI (BGR memorizzato nei
tre byte bassi), quindi la disposizione dei byte è `&hBBGGRR`:

| Colore  | Costante nuBASIC |
|---------|-----------------|
| Rosso   | `&h0000FF`      |
| Verde   | `&h00FF00`      |
| Blu     | `&hFF0000`      |
| Giallo  | `&h00FFFF`      |
| Ciano   | `&hFFFF00`      |
| Magenta | `&hFF00FF`      |
| Bianco  | `&hFFFFFF`      |
| Nero    | `&h000000`      |

La funzione `Rgb(r, g, b)` calcola un valore colore a partire dalle componenti rosso, verde e blu
comprese tra 0 e 255, risultando spesso più chiara rispetto a un letterale esadecimale grezzo:

```basic
skyBlue%  = Rgb(135, 206, 235)
sunYellow% = Rgb(255, 220, 50)
```

### 5.1 Primitive di Disegno

#### Linee e forme

```basic
' Draw a straight line from (x1,y1) to (x2,y2)
Line x1, y1, x2, y2, color%

' Rectangle outline
Rect x1, y1, x2, y2, color%

' Filled rectangle (the most common primitive — use for backgrounds and clearing regions)
FillRect x1, y1, x2, y2, color%

' Ellipse outline (bounding box given by the two corner points)
Ellipse x1, y1, x2, y2, color%

' Filled ellipse
FillEllipse x1, y1, x2, y2, color%
```

#### Pixel

I pixel singoli possono essere scritti e letti. `SetPixel` è il mattone fondamentale per renderer
personalizzati come la demo Mandelbrot in `rosetta.bas`.

```basic
SetPixel x%, y%, color%       ' write a pixel
c% = GetPixel(x%, y%)         ' read a pixel's color
```

#### Testo sullo schermo

`TextOut` disegna una stringa di testo a una coordinata pixel usando il font corrente della console. A
differenza di `Print`, che sposta il cursore testuale e fa scorrere il buffer, `TextOut` disegna
direttamente sulla superficie grafica in una posizione fissa e non influisce sul cursore di testo.

```basic
TextOut x%, y%, text$, color%

' Examples:
TextOut 10,  10, "Score: " + Str$(score%), &hFFFFFF
TextOut 200, 240, "GAME OVER", Rgb(255, 50, 50)
```

#### Immagini bitmap

Ci sono due modi per disegnare una bitmap. Conviene usare la API in cache
ogni volta che la stessa immagine viene disegnata piu' di una volta
(sprite, tile, frame di animazione): il file viene decodificato solo alla
prima chiamata e i disegni successivi usano un handle GDI/GPU.

##### API bitmap in cache (preferita)

| Funzione | Restituisce | Descrizione |
|----------|-------------|-------------|
| `BitmapLoad(percorso$)` | Integer | Carica un BMP/PNG/JPEG dal disco e restituisce un handle intero |
| `BitmapDraw(id, x, y)` | Integer | Disegna una bitmap precedentemente caricata alle coordinate pixel in alto a sinistra |
| `BitmapFree(id)` | Integer | Libera la bitmap dalla cache |

```basic
Dim sprite As Integer
sprite = BitmapLoad("hero.bmp")
For frame% = 1 To 240
   Cls
   BitmapDraw sprite, hero_x%, hero_y%
   Refresh
Next frame%
BitmapFree sprite
```

La cache usa GDI+ su Windows e stb_image su Linux (i canali R/B vengono
pre-scambiati al caricamento perche' il costo per frame sia equivalente).
`BitmapLoad` restituisce 0 in caso di errore.

##### Disegno una tantum (`PlotImage`)

`PlotImage` e' il punto d'ingresso storico: ricarica il file dal disco a
ogni chiamata. E' utile per disegni occasionali, ma andrebbe evitato nei
loop di animazione.

```basic
PlotImage "background.bmp", 0, 0           ' va bene per uno splash statico
PlotImage "sprite.bmp", hero_x%, hero_y%   ' spreco dentro un loop
```

#### Esempio di scena completa

```basic
Cls
MoveWindow GetWindowX(), GetWindowY(), 640, 480
FillRect 0, 0, 640, 480, Rgb(30, 30, 60)        ' dark night sky
FillEllipse 520, 20, 600, 100, Rgb(255, 255, 200) ' moon
FillRect 0, 360, 640, 480, Rgb(0, 80, 0)         ' green ground
For i% = 0 To 5
   cx% = 80 + i% * 100
   FillEllipse cx%, 200, cx%+60, 360, Rgb(20, 120, 20)  ' trees
   FillRect cx%+25, 330, cx%+35, 365, Rgb(80, 50, 20)   ' trunks
Next i%
TextOut 10, 10, "nuBASIC Graphics Demo", &hFFFFFF
```

### 5.2 Rendering senza sfarfallio

#### Il problema

Quando un programma richiama più comandi di disegno in sequenza — azzerare lo sfondo, disegnare
il tabellone, disegnare il punteggio, disegnare lo sprite del giocatore — ogni singola chiamata effettua
immediatamente un blit del back buffer verso lo schermo visibile. L'utente vede brevemente ogni stato
intermedio: un fotogramma nero azzerato, poi un tabellone parziale, poi il tabellone con il punteggio, e
infine il fotogramma completo. Questo si manifesta come uno sfarfallio visibile, particolarmente evidente ad
alta frequenza di aggiornamento o con scene complesse.

#### La soluzione: double buffering

`ScreenLock` indica al renderer di accumulare tutti i comandi di disegno successivi in un back buffer
fuori schermo senza mostrare nulla a video. Quando viene chiamato `ScreenUnlock`, il back buffer completo
viene trasferito allo schermo in un'unica operazione atomica — l'utente vede soltanto il fotogramma
finito, mai gli stati intermedi. `Refresh` esegue lo stesso blit su richiesta senza modificare lo stato del
lock.

| Istruzione | Effetto |
|-------------|--------|
| `ScreenLock` | Sospende l'aggiornamento automatico dello schermo; tutti i disegni vanno al back buffer |
| `ScreenUnlock` | Presenta il back buffer sullo schermo in un solo blit; riprende l'aggiornamento automatico |
| `Refresh` | Forza un blit immediato; lo stato del lock rimane invariato |

#### Schema 1 — ciclo di fotogrammi di gioco

Questo è lo schema canonico per qualsiasi programma animato. Ogni fotogramma viene composto
interamente nel back buffer e presentato come un'unica unità:

```basic
While Not(game_over%)
   ScreenLock
   FillRect 0, 0, 640, 480, &h000000   ' clear: erase previous frame
   DrawBoard                            ' render game board
   DrawPlayer player_x%, player_y%      ' render player
   DrawEnemies                          ' render all enemies
   DrawHUD score%, lives%               ' render heads-up display
   ScreenUnlock
   MDelay 16                            ' pace to ~60 fps
Wend
```

#### Schema 2 — rendering progressivo (Mandelbrot / frattale)

Per render computazionalmente onerosi in cui si vuole mostrare l'avanzamento, si usa `ScreenLock`
al livello del ciclo esterno in modo che ogni colonna (o riga) venga presentata atomicamente anziché
pixel per pixel:

```basic
For x0 = -2 To 2 Step 0.013
   ScreenLock
   For y0 = -1.5 To 1.5 Step 0.013
      ' ... compute iteration count c ...
      FillRect x0*d%+dx%, y0*d%+dy%, x0*d%+dx%+2, y0*d%+dy%+2, c%*16
   Next y0
   ScreenUnlock       ' one column appears per iteration — smooth progressive reveal
Next x0
```

#### Schema 3 — cancellazione + ridisegno atomici (oggetto in movimento)

Per spostare un oggetto senza tracce fantasma, si cancella la vecchia posizione e si disegna la nuova
posizione all'interno di un singolo lock:

```basic
' Compute new position
x_old% = x%  :  y_old% = y%
x% = x% + dx%  :  y% = y% + dy%

ScreenLock
FillEllipse x_old%*10, y_old%*10, x_old%*10+10, y_old%*10+10, 0        ' erase old
FillEllipse x%*10,     y%*10,     x%*10+10,     y%*10+10,     &hffffff ' draw new
ScreenUnlock
```

#### Schema 4 — finestra di dialogo prima di un input bloccante

Prima di attendere un tasto con `Input$()`, si usa `Refresh` per assicurarsi che la finestra di dialogo
sia visibile. Senza di esso, se un `ScreenLock` è attivo altrove nel programma, l'utente potrebbe non
vedere la finestra:

```basic
ScreenLock
FillRect 150, 220, 490, 310, &hffff00
Rect     150, 220, 490, 310, &h000000
TextOut  180, 245, "Game over! Play again? (Y/N)", &h000000
ScreenUnlock
Refresh
key$ = Input$(1)
```

### 5.3 Input del Mouse

I programmi nuBASIC possono leggere la posizione del mouse e lo stato dei pulsanti in qualsiasi
momento. L'input del mouse viene acquisito per polling — non esiste una coda di eventi — quindi
l'utilizzo tipico è chiamare `GetMouse()` una volta per fotogramma all'interno del ciclo
principale e agire sui campi della struct restituita.

#### GetMouse() — preferita

`GetMouse()` acquisisce posizione e stato dei pulsanti in una sola chiamata e restituisce una
struct `Mouse`. Il tipo `Mouse` è preregistrato dall'interprete, quindi non è necessaria alcuna
definizione `Struct`:

```basic
Dim m As Mouse
m = GetMouse()
' m.x   — X del cursore in pixel dal bordo sinistro della finestra
' m.y   — Y del cursore in pixel dal bordo superiore della finestra
' m.btn — 0=nessuno, 1=sinistro, 2=centrale, 4=destro
```

#### Funzioni scalari legacy *(deprecate — rimosse in v2.0)*

```basic
btn% = GetMouseBtn()   ' usa invece GetMouse().btn
x%   = GetMouseX()     ' usa invece GetMouse().x
y%   = GetMouseY()     ' usa invece GetMouse().y
```

#### Hit-test di una regione pulsante

L'uso più comune dell'input del mouse è verificare se il cursore si trova all'interno di una regione
rettangolare nel momento in cui un pulsante viene premuto:

```basic
' Definisci un rettangolo pulsante
bx1% = 40  :  by1% = 60
bx2% = 200 :  by2% = 100

' Disegna il pulsante
FillRect bx1%, by1%, bx2%, by2%, &hffff00
TextOut bx1%+20, by1%+15, "Click me", &h000000

' Ciclo principale di interazione
Dim m As Mouse
While 1
   m = GetMouse()

   If m.btn = 1 And m.x >= bx1% And m.x <= bx2% And m.y >= by1% And m.y <= by2% Then
      Print "Button clicked!"
      MDelay 200    ' antirimbalzo — attende il rilascio
   End If

   MDelay 16
Wend
```

#### Disegno con il mouse

```basic
Cls
FillRect 0, 0, 640, 480, &h000000
TextOut 10, 10, "Hold left button and draw. Press Q to quit.", &hffffff

Dim m As Mouse
While 1
   key$ = InKey$()
   If key$ = "q" Or key$ = "Q" Then Exit While

   m = GetMouse()
   If m.btn = 1 Then
      SetPixel m.x, m.y, &hffffff
   End If

   MDelay 5
Wend
```

### 5.4 Audio e Gestione della Finestra

#### Controllo della finestra

La finestra di lavoro di nuBASIC può essere posizionata e ridimensionata dal codice BASIC. Ciò è utile
per impostare le dimensioni corrette della tela all'avvio di un programma grafico, o per implementare
layout in stile schermo intero.

```basic
' Resize and position the window
MoveWindow GetWindowX(), GetWindowY(), 800, 600

' Query current window geometry
Print "Position: "; GetWindowX(); ", "; GetWindowY()
Print "Size:     "; GetWindowDx(); " x "; GetWindowDy()

' Query the drawable client area (may differ from window size due to title bar / borders)
Print "Canvas: "; GetSWidth(); " x "; GetSHeight()

' Keep the window above all others
SetTopMost()
```

Una tipica sequenza di avvio del programma:

```basic
Cls
MoveWindow 100, 100, 640, 480
FillRect 0, 0, 640, 480, &h000000   ' clear to black after resize
```

#### Audio

`PlaySound` riproduce un file audio WAV. Il secondo argomento controlla la riproduzione sincrona o
asincrona: 0 blocca fino al termine del suono, 1 ritorna immediatamente in modo che il programma
continui a girare mentre il suono viene riprodotto.

```basic
PlaySound "background.wav", 1   ' async: music plays in background
PlaySound "explosion.wav", 0    ' sync: wait for it to finish
PlaySound "coin.wav", 1
```

`Beep` produce il segnale acustico predefinito del sistema — utile per un feedback semplice nei giochi
o come indicatore di errore:

```basic
Beep   ' simple system beep
```

#### Finestre di messaggio

`MsgBox` visualizza una finestra di dialogo modale che sospende l'esecuzione del programma fino a
quando l'utente non la chiude. È utile per avvisi, messaggi di errore e semplici richieste di
conferma sì/no.

```basic
result% = MsgBox("nuBASIC Demo", "Setup complete. Ready to play?")
If result% > 0 Then
   Print "User confirmed."
End If
```

---

### 5.5 Pseudo-3D con WinRayCast

Su Windows, nuBASIC 2.0 integra **WinRayCast**, un motore di raycasting leggero che renderizza
scene in prima persona pseudo-3D, in stile *Wolfenstein 3D*, a partire da una semplice mappa a
griglia 2D. WinRayCast gestisce il rendering e la logica di gioco (pareti texturizzate, sprite,
porte, attori, armi, oggetti raccoglibili, audio e transizioni tra livelli); il programma BASIC
lo pilota attraverso il modulo `raycast` — le funzioni integrate `Ray…` — e fornisce la gestione
dell'input, il disegno dell'HUD e le regole di gioco.

> **Piattaforma.** WinRayCast è solo per Windows. Il modulo è compilato per impostazione
> predefinita nelle build Windows (`NUBASIC_WITH_RAYCAST`) ed è assente altrove, dove esiste
> solo `RayAvailable()` e restituisce `0`. Proteggi il codice portabile con
> `If RayAvailable() Then …`.

#### Il ciclo di rendering

Un programma WinRayCast si inizializza una volta, carica un mondo o un progetto, poi ripete un
ciclo **input → aggiornamento → rendering → presentazione** e termina alla fine. `Screen 1` deve
essere attivo, perché i fotogrammi vengono presentati nella finestra GDI.

```basic
Syntax Modern
Using raycast
Using graphics

Const VIEW_W = 640
Const VIEW_H = 400

If RayAvailable() = 0 Then Print "WinRayCast not available" : End

Screen 1
RaySetBaseDir(GetAppPath())                 ' risolve gli asset rispetto all'exe
RayInit(VIEW_W, VIEW_H)                     ' crea la sessione / il framebuffer
RayLoadProject("raycast_demo/worlds/demo.world.json")

Dim running As Integer : running = 1
While running
   ScreenLock
   If RayKeyDown(27) Then running = 0       ' ESC
   If RayKeyDown(38) Then RayMove(8)        ' freccia Su
   If RayKeyDown(40) Then RayMove(-8)       ' freccia Giù
   If RayKeyDown(37) Then RayTurn(-3)       ' freccia Sinistra
   If RayKeyDown(39) Then RayTurn(3)        ' freccia Destra

   RayUpdate(0.016)                         ' avanza ~16 ms di tempo del mondo
   RayRender(VIEW_W, VIEW_H)                ' renderizza nel framebuffer
   RayPresent(0, 0, VIEW_W, VIEW_H)         ' blit del framebuffer su (x, y, w, h)
   ScreenUnlock
   MDelay 16                                ' regola a ~60 fps
Wend

RayShutdown()
```

`RayUpdate(dt)` fa avanzare porte, animazioni degli sprite, attori/nemici, oggetti raccoglibili,
effetti e l'oscillazione dell'arma (`dt` è internamente limitato a 0,1 s). `RayRender` riempie un
framebuffer fuori schermo e `RayPresent` lo copia in un rettangolo della finestra — il disegno
dell'HUD e della minimappa con le normali primitive grafiche può essere sovrapposto prima di
`ScreenUnlock`.

#### Mondi e progetti

I dati delle scene vengono caricati da file JSON ancorati a una directory di base
(`RaySetBaseDir`): `RayLoadWorld(path$)` carica una singola mappa `*.world.json`, mentre
`RayLoadProject(path$)` carica un progetto multi-livello con transizioni a ascensore tra i
livelli. La griglia della mappa può essere ispezionata con `RayCellKind` (0 vuoto, 1 parete,
2 porta), `RayIsSolidCell` e `RayKeyAtCell`, utili per disegnare una minimappa.

#### Funzioni `Ray…` principali

| Gruppo | Funzioni |
|-------|-----------|
| Ciclo di vita | `RayAvailable`, `RayInit`, `RaySetBaseDir`, `RayShutdown` |
| Mondo / progetto | `RayLoadWorld`, `RayLoadProject`, `RayCurrentLayer$` |
| Fotogramma | `RayUpdate`, `RayRender`, `RayPresent`, `RayFrameHash` |
| Giocatore / camera | `RayPlayerX`, `RayPlayerY`, `RayPlayerFacing`, `RaySetPlayer`, `RayMove`, `RayStrafe`, `RayTurn`, `RayPlayerSlope`/`RaySetPlayerSlope`, `RayPlayerViewCenter`/`RaySetPlayerViewCenter` |
| Mappa | `RayMapRows`, `RayMapCols`, `RayCellDx`, `RayCellDy`, `RayIsSolidCell`, `RayCellKind`, `RayKeyAtCell`, `RayMapUnlockCount` |
| Sprite / avanzamento | `RaySpriteCount`, `RayActorCount`, `RayEnemyCount`, `RayKilledEnemyCount`, `RayItemCount`, `RayCollectedItemCount`, `RayDestroyedObjectCount`, `RayPlayerStandingOn` |
| Armi | `RayLoadWeapon`, `RayHasWeapon`, `RayWeaponAmmo`, `RayWeaponReserveAmmo`, `RayWeaponMaxAmmo`, `RayReloadWeapon` |
| Combattimento / audio | `RayDamageEnemy`, `RayConsumePlayerDamage`, `RayConsumePlayerHealing`, `RaySetPlayerEnergy`, `RayPlaySound` |
| Input | `RayKeyDown` |
| Transizioni | `RayTransitionActive`, `RaySetTransitionManual`, `RayTransitionOptionCount`, `RayTransitionOptionLayer$`, `RayTransitionSelected`, `RaySelectTransition`, `RayConfirmTransition` |

Il riferimento completo per ciascuna funzione (firme, valori di ritorno e semantica) si trova
nella pagina wiki
[Raycast Game Engine](https://github.com/eantcal/nubasic/wiki/Raycast-Game-Engine). Per un
esempio completo e giocabile vedi `examples/raycast/eclipse_protocol.bas` — una demo FPS in
stile tutorial con combattimento multi-arma, energia/salute, un HUD e una minimappa, porte con
chiave/serratura, checkpoint con salvataggio automatico e transizioni a ascensore, che mostra
anche `Syntax Modern`, `Struct` e `Class` che lavorano insieme.

---

## 6. Riferimento ai Comandi

### 6.1 Comandi della Console

Questi comandi sono disponibili nel REPL interattivo. Controllano l'interprete stesso —
caricamento, salvataggio, debug e ispezione dei programmi — e non sono validi all'interno di un
programma BASIC in esecuzione.

| Comando | Descrizione | Sintassi |
|---------|-------------|--------|
| `Run` | Esegue il programma in memoria | `Run [linenumber]` |
| `Cont` | Riprende dopo un `Stop` o un breakpoint | `Cont` |
| `Resume` | Riprende dalla riga successiva all'interruzione | `Resume` |
| `Break` | Imposta un breakpoint, facoltativamente condizionale | `Break [[linenum] [If expr]]` |
| `RmBrk` | Rimuove un breakpoint | `RmBrk linenum` |
| `ClrBrk` | Rimuove tutti i breakpoint | `ClrBrk` |
| `Load` | Carica un file di programma in memoria | `Load filename` |
| `Save` | Salva il programma corrente su file | `Save filename` |
| `Exec` | Carica ed esegue un file di programma | `Exec filename` |
| `List` | Elenca tutto o parte del programma | `List [from[-to]]` |
| `New` | Azzera il programma e tutte le variabili | `New` |
| `Clr` | Azzera tutte le variabili (mantiene il programma) | `Clr` |
| `Renum` | Rinumera le righe del programma | `Renum [increment]` |
| `Grep` | Elenca le righe che corrispondono a un pattern | `Grep pattern` |
| `Vars` | Mostra le variabili correnti a runtime | `Vars` |
| `Meta` | Mostra i metadati di compilazione | `Meta` |
| `Ver` | Stampa la versione dell'interprete | `Ver` |
| `TrOn` | Abilita il tracciamento del programma | `TrOn` |
| `TrOff` | Disabilita il tracciamento del programma | `TrOff` |
| `StOn` | Abilita l'esecuzione passo per passo | `StOn` |
| `StOff` | Disabilita l'esecuzione passo per passo | `StOff` |
| `Pwd` | Stampa la directory di lavoro corrente | `Pwd` |
| `Cd` | Cambia la directory di lavoro | `Cd path` |
| `Help` | Mostra la guida per una parola chiave | `Help [keyword]` |
| `Apropos` | Cerca nelle descrizioni della guida | `Apropos keyword` |
| `!` | Esegue un comando di shell | `! shell_command` |
| `Exit` | Chiude l'interprete | `Exit` |

### 6.2 Istruzioni

#### Struttura del programma

| Istruzione | Sintassi | Descrizione |
|-------------|--------|-------------|
| `Dim` | `Dim var[(n)] [As Type]` | Dichiara una variabile o un array |
| `ReDim` | `ReDim var(n)` | Ridimensiona un array (azzera il contenuto) |
| `Const` | `Const name [As Type] = value` | Dichiara una costante |
| `Let` | `[Let] var = expr` | Assegna un valore (parola chiave facoltativa) |
| `Sub` | `Sub name(params)` … `End Sub` | Definisce una subroutine |
| `Function` | `Function name(params) [As Type]` … `End Function` | Definisce una funzione |
| `Struct` | `Struct name` … `End Struct` | Definisce un tipo composito |
| `End` | `End` | Arresta l'esecuzione del programma |
| `Stop` | `Stop` | Entra nel REPL (riprendere con `Cont`) |

#### Flusso di controllo

| Istruzione | Sintassi | Descrizione |
|-------------|--------|-------------|
| `If` | `If expr Then … [ElIf expr Then …] [Else …] End If` | Ramo condizionale |
| `For` | `For var=x To y [Step z]` … `Next [var]` | Ciclo contato |
| `While` | `While cond` … `Wend \| End While` | Ciclo con precondizione |
| `Do` | `Do` … `Loop While cond` | Ciclo con postcondizione |
| `Exit For` | `Exit For` | Esce dal ciclo For |
| `Exit While` | `Exit While` | Esce dal ciclo While |
| `Exit Do` | `Exit Do` | Esce dal ciclo Do |
| `Exit Sub` | `Exit Sub` | Ritorna anticipatamente dalla Sub |
| `Exit Function` | `Exit Function` | Ritorna anticipatamente dalla Function |
| `GoTo` | `GoTo label \| linenum` | Salto incondizionato |
| `GoSub` | `GoSub label \| linenum` | Chiama una subroutine |
| `Return` | `Return` | Ritorna dal GoSub |
| `On` | `On expr GoTo label1[, label2, …]` | Ramo calcolato |

#### I/O

| Istruzione | Sintassi | Descrizione |
|-------------|--------|-------------|
| `Print` | `Print [expr[; \| , …]]` | Output sulla console |
| `Write` | `Write [expr[; \| , …]]` | Output senza newline finale |
| `Print#` | `Print# filenum, expr` | Output su file |
| `Input` | `Input [prompt;] var` | Lettura dalla console |
| `Input#` | `Input# filenum, var` | Lettura da file |
| `Read#` | `Read# filenum, var, size` | Lettura binaria grezza |
| `Open` | `Open filename For mode As #n` | Apre un file |
| `FOpen` | `FOpen filename, mode$, n` | Apre un file (stringa di modalità in stile C) |
| `Close` | `Close #n` | Chiude un file |
| `Seek` | `Seek filenum, pos, origin` | Sposta il puntatore del file |
| `Flush` | `Flush filenum` | Scarica il buffer del file su disco |
| `Locate` | `Locate row, col` | Sposta il cursore testuale (base 1) |
| `Cls` | `Cls` | Cancella lo schermo della console |
| `Beep` | `Beep` | Emette il segnale acustico di sistema |

#### Temporizzazione

| Istruzione | Sintassi | Descrizione |
|-------------|--------|-------------|
| `Delay` / `Sleep` | `Delay seconds` | Sospende l'esecuzione (in secondi) |
| `MDelay` | `MDelay milliseconds` | Sospende l'esecuzione (in millisecondi) |

#### Dati

| Istruzione | Sintassi | Descrizione |
|-------------|--------|-------------|
| `Data` | `Data expr1[, expr2, …]` | Memorizza valori di dati costanti |
| `Read` | `Read var1[, var2, …]` | Legge dall'archivio Data |
| `Restore` | `Restore [index]` | Riavvolge o riposiziona il puntatore Data |

#### Grafica

| Istruzione | Sintassi | Descrizione |
|-------------|--------|-------------|
| `Line` | `Line x1,y1,x2,y2,color` | Disegna una linea |
| `Rect` | `Rect x1,y1,x2,y2,color` | Disegna il contorno di un rettangolo |
| `FillRect` | `FillRect x1,y1,x2,y2,color` | Disegna un rettangolo pieno |
| `Ellipse` | `Ellipse x1,y1,x2,y2,color` | Disegna il contorno di un'ellisse |
| `FillEllipse` | `FillEllipse x1,y1,x2,y2,color` | Disegna un'ellisse piena |
| `SetPixel` | `SetPixel x,y,color` | Disegna un singolo pixel |
| `TextOut` | `TextOut x,y,text$,color` | Disegna testo alle coordinate pixel |
| `PlotImage` | `PlotImage bitmap$,x,y` | Disegna un'immagine dal disco (legacy, ricarica ogni volta) |
| `BitmapLoad` | `id = BitmapLoad(file$)` | Carica una bitmap nella cache e restituisce un handle |
| `BitmapDraw` | `BitmapDraw id,x,y` | Disegna una bitmap dalla cache (preferita per animazioni) |
| `BitmapFree` | `BitmapFree id` | Libera una bitmap dalla cache |
| `ScreenLock` | `ScreenLock` | Sospende l'aggiornamento dello schermo (inizio composizione fotogramma) |
| `ScreenUnlock` | `ScreenUnlock` | Presenta il back buffer sullo schermo (fine fotogramma) |
| `Refresh` | `Refresh` | Forza un blit immediato (stato del lock invariato) |

#### Varie

| Istruzione | Sintassi | Descrizione |
|-------------|--------|-------------|
| `ChDir` | `ChDir path$` | Cambia la directory di lavoro |
| `Shell` | `Shell "command"` | Esegue un comando di shell |
| `Randomize` | `Randomize` | Inizializza il generatore di numeri casuali con l'ora corrente |

### 6.3 Funzioni Integrate

#### Matematica

| Funzione | Restituisce | Descrizione |
|----------|---------|-------------|
| `Abs(x)` | Double | Valore assoluto |
| `Int(x)` | Integer | Troncamento verso zero |
| `TruncF(x)` | Double | Arrotondamento verso zero (risultato double) |
| `Sign(x)` | Integer | −1, 0 oppure 1 |
| `Sqr(x)` / `Sqrt(x)` | Double | Radice quadrata |
| `Pow(x,y)` | Double | x elevato alla potenza y |
| `Exp(x)` | Double | e^x |
| `Log(x)` | Double | Logaritmo naturale |
| `Log10(x)` | Double | Logaritmo in base 10 |
| `Sin(x)` | Double | Seno (x in radianti) |
| `Cos(x)` | Double | Coseno (x in radianti) |
| `Tan(x)` | Double | Tangente (x in radianti) |
| `ASin(x)` | Double | Arcoseno (risultato in radianti) |
| `ACos(x)` | Double | Arcocoseno (risultato in radianti) |
| `ATan(x)` | Double | Arcotangente (risultato in radianti) |
| `Sinh(x)` | Double | Seno iperbolico |
| `Cosh(x)` | Double | Coseno iperbolico |
| `Tanh(x)` | Double | Tangente iperbolica |
| `PI()` | Double | π ≈ 3.14159265358979 |
| `Rnd(x)` | Double | Numero casuale in [0,1); x<0 reinizializza il generatore |
| `Min(x,y)` | Double | Il minore tra x e y |
| `Max(x,y)` | Double | Il maggiore tra x e y |
| `Conv(v1,v2[,c1,c2])` | Array | Convoluzione discreta di due vettori numerici |

#### Stringhe

| Funzione | Restituisce | Descrizione |
|----------|---------|-------------|
| `Len(s$)` | Integer | Lunghezza in caratteri |
| `Left$(s$,n)` | String | I primi n caratteri |
| `Right$(s$,n)` | String | Gli ultimi n caratteri |
| `Mid$(s$,n,m)` | String | m caratteri dalla posizione n (base 1) |
| `SubStr$(s$,n,m)` | String | m caratteri dall'offset n (base 0) |
| `PStr$(s$,pos,c$)` | String | Restituisce s$ con il carattere alla posizione pos (base 0) sostituito da c$ |
| `InStr(s$,t$)` | Integer | Posizione (base 1) di t$ in s$; −1 se assente (senza distinzione maiuscole/minuscole) |
| `InStrCS(s$,t$)` | Integer | Come sopra ma con distinzione maiuscole/minuscole |
| `UCase$(s$)` | String | Converte in maiuscolo |
| `LCase$(s$)` | String | Converte in minuscolo |
| `Str$(x)` | String | Numero in stringa |
| `StrP$(x,p%)` | String | Numero in stringa con p% cifre significative |
| `Hex$(x)` | String | Rappresentazione esadecimale |
| `Val(s$)` | Double | Interpreta la stringa come numero in virgola mobile |
| `Val%(s$)` | Integer | Interpreta la stringa come numero intero |
| `Asc(s$)` | Integer | Punto di codice Unicode del primo carattere |
| `Chr$(n)` | String | Carattere dal punto di codice Unicode |
| `Spc(n)` | String | Stringa di n spazi |
| `Eval(s$)` | Any | Valuta una stringa di espressione nuBASIC a runtime |

#### Tastiera / Input

| Funzione | Restituisce | Descrizione |
|----------|---------|-------------|
| `InKey$()` | String | Lettura del tasto non bloccante; stringa vuota se nessun tasto è in attesa |
| `Input$(n)` | String | Blocca fino alla lettura di esattamente n caratteri |
| `GetVKey()` | Integer | Legge il prossimo codice tasto virtuale dall'input |

#### File

| Funzione | Restituisce | Descrizione |
|----------|---------|-------------|
| `EOF(n)` | Integer | 1 se il puntatore del file è alla fine; 0 altrimenti |
| `FError(n)` | Integer | Diverso da zero se si è verificato un errore sul file n |
| `FTell(n)` | Integer | Offset corrente in byte nel file n |
| `FSize(n)` | Integer | Dimensione totale in byte del file n |
| `Errno()` | Integer | Codice di errore di sistema dell'ultima operazione fallita |
| `Errno$(n)` | String | Descrizione leggibile per il codice di errore n |
| `MkDir(path$)` | Integer | Crea una directory; restituisce 0 in caso di successo |
| `RmDir(path$)` | Integer | Rimuove una directory; restituisce 0 in caso di successo |
| `Erase(path$)` | Integer | Elimina un file; restituisce 0 in caso di successo |
| `Pwd$()` | String | Directory di lavoro corrente |
| `GetEnv$(var$)` | String | Valore di una variabile d'ambiente |
| `SetEnv(var$,val$)` | Integer | Imposta una variabile d'ambiente |
| `UnsetEnv(var$)` | Integer | Rimuove una variabile d'ambiente |

#### Sistema / Ora

| Funzione | Restituisce | Descrizione |
|----------|---------|-------------|
| `GetDateTime()` | struct DateTime | Tutti i campi di data/ora: `year`, `month`, `day`, `hour`, `minute`, `second`, `wday`, `yday` |
| `SysTime$()` | String | Ora e data locali correnti come stringa |
| `Time()` | Integer | Secondi trascorsi dall'Epoca Unix |
| `Millis()` | Integer | Millisecondi da un istante di riferimento fisso (timing / cadenza dei frame) |
| `GetPlatId()` | Integer | 1 = Windows, 2 = Linux/altro |
| `GetAppPath$()` | String | Percorso completo dell'eseguibile nuBASIC |
| `Ver$()` | String | Stringa di versione di nuBASIC |

> **Rimosse in v2.0.** Le funzioni scalari di data/ora `SysHour`, `SysMin`, `SysSec`,
> `SysDay`, `SysMonth`, `SysYear`, `SysWDay` e `SysYDay` (deprecate nella v1.62) sono state
> rimosse. Usa il campo corrispondente di `GetDateTime()` (per es. `GetDateTime().hour`).

#### Grafica / Finestra (solo versione completa)

| Funzione | Restituisce | Descrizione |
|----------|---------|-------------|
| `Rgb(r,g,b)` | Integer | Compone un colore RGB da componenti 0–255 |
| `GetPixel(x,y)` | Integer | Legge il colore di un pixel |
| `GetMouse()` | struct Mouse | Tutto lo stato del puntatore: `x`, `y`, `btn` |
| `GetSWidth()` | Integer | Larghezza dell'area client disegnabile in pixel |
| `GetSHeight()` | Integer | Altezza dell'area client disegnabile in pixel |
| `GetWindowX()` | Integer | Posizione del bordo sinistro della finestra sullo schermo |
| `GetWindowY()` | Integer | Posizione del bordo superiore della finestra sullo schermo |
| `GetWindowDx()` | Integer | Larghezza totale della finestra in pixel |
| `GetWindowDy()` | Integer | Altezza totale della finestra in pixel |
| `MoveWindow(x,y,w,h)` | Integer | Sposta e ridimensiona la finestra di lavoro |
| `SetTopMost()` | Integer | Rende la finestra sempre in primo piano |
| `MsgBox(title$,msg$)` | Integer | Mostra una finestra di dialogo modale; restituisce > 0 se confermata |
| `PlaySound(file$,async%)` | Integer | Riproduce un file WAV (async%=1 ritorna immediatamente) |

> **Rimosse in v2.0.** Gli accessori scalari del mouse `GetMouseX`, `GetMouseY` e
> `GetMouseBtn` (deprecati nella v1.62) sono stati rimossi; usa `GetMouse()`.

#### Raycast (Windows)

Il motore WinRayCast integrato aggiunge una famiglia di funzioni `Ray…` (`RayInit`,
`RayLoadProject`, `RayUpdate`, `RayRender`, `RayPresent` e le query
giocatore/combattimento/mappa) nelle build Windows. Sono documentate per esteso nella
[sezione 5.5](#55-pseudo-3d-con-winraycast).

#### Tabelle Hash

| Funzione | Restituisce | Descrizione |
|----------|---------|-------------|
| `HSet(hash$,key$,val)` | — | Inserisce o aggiorna una voce |
| `HGet(hash$,key$)` | Any | Recupera il valore per una chiave |
| `HChk(hash$,key$)` | Boolean | Vero se la chiave esiste nella tabella |
| `HCnt(hash$)` | Integer | Numero di voci nella tabella |
| `HDel(hash$[,key$])` | — | Elimina una singola chiave, o l'intera tabella se la chiave è omessa |

#### Varie

| Funzione | Restituisce | Descrizione |
|----------|---------|-------------|
| `Not(x)` | Integer | NOT logico: 1 se x=0, 0 altrimenti |
| `bNot(x)` | Integer | NOT bit a bit di x |
| `SizeOf(x)` | Integer | Dimensione in byte di una variabile o dell'intero array |
| `SizeOf@(x)` | Integer | Numero di elementi in un array |

### 6.4 Riferimento agli Operatori

| Operatore | Descrizione | Esempio |
|----------|-------------|---------|
| `+` | Somma numeri o concatena stringhe | `a + b`, `"Hi" + name$` |
| `-` | Sottrazione; negazione unaria | `a - b`, `-x` |
| `*` | Moltiplicazione | `a * b` |
| `/` | Divisione (risultato in virgola mobile) | `a / b` |
| `\` o `Div` | Divisione intera (troncamento) | `a \ b` |
| `Mod` | Modulo — resto dopo la divisione intera | `a Mod b` |
| `^` | Elevamento a potenza | `2 ^ 10` |
| `++` | Incremento prefisso | `++i%` |
| `--` | Decremento prefisso | `--i%` |
| `=` | Uguale (confronto) o assegnazione | `a = b` |
| `<>` | Diverso da | `a <> b` |
| `<` `>` `<=` `>=` | Confronto ordinato | `a < b` |
| `And` | AND logico | `a And b` |
| `Or` | OR logico | `a Or b` |
| `Xor` | OR esclusivo logico | `a Xor b` |
| `Not` | NOT logico | `Not(x)` |
| `bAnd` | AND bit a bit | `a bAnd b` |
| `bOr` | OR bit a bit | `a bOr b` |
| `bXor` | XOR bit a bit | `a bXor b` |
| `bNot` | NOT bit a bit | `bNot(x)` |
| `bShl` | Scorrimento a sinistra | `a bShl 2` |
| `bShr` | Scorrimento a destra | `a bShr 2` |

---

## 7. Ambiente di Sviluppo Integrato (IDE)

L'IDE di nuBASIC racchiude in un'unica applicazione un editor avanzato con evidenziazione della
sintassi, un interprete e un debugger, disponibile per Windows e Linux (GTK+2). È lo strumento
consigliato per scrivere programmi non banali — mentre l'editor a schermo del CLI è sufficiente per
script brevi e sperimentazione, l'IDE offre l'esperienza di sviluppo completa.

#### Novita' nell'IDE

L'IDE Windows ha ricevuto tre cicli significativi di miglioramenti:

- **Debugger piu' ricco.** Il debugger espone ora il set completo di
  comandi di stepping tipici degli IDE piu' grandi — **Step Into**,
  **Step Over**, **Step Out**, **Esegui fino al cursore**, **Pausa /
  Break** — oltre al flusso storico Continua / Passo / Toggle
  Breakpoint. I distruttori di classe, le chiamate `Sub`/`Function`
  dentro le espressioni e i programmi multi-sorgente vengono debuggati
  correttamente. Vedi [§7.9](#79-debugger-integrato).
- **Progetti multi-sorgente.** Un piccolo formato di file di progetto
  (`.nbp`) registra il punto d'ingresso, la modalita' di sintassi e il
  nome del progetto; aprendo un `.nbp` nell'IDE viene caricato lo
  script di entry e il debugger entra nei file `Include` come se fosse
  un unico programma. Vedi [§7.11](#711-progetti-multi-sorgente).
- **Supporto Visual Studio Code.** Un'estensione VS Code di prima
  classe e' distribuita accanto all'IDE nativo: offre evidenziazione
  della sintassi, Run/Debug, breakpoint, watch, call stack e gli
  stessi comandi di stepping tramite la UI di debug standard di VS
  Code. L'estensione usa `nubasicdebug` come backend
  console-subsystem, integrandosi in modo pulito con l'editor. Vedi
  [§7.10](#710-estensione-vs-code).

Queste aggiunte si stratificano sopra l'editor esistente e non
modificano il workflow storico: chi e' a proprio agio con i tasti
originali `F5` / `F8` / `F9` / `F10` puo' continuare a usarli senza
modifiche.

---

### 7.1 Editor con Evidenziazione della Sintassi

L'editor è basato sulla libreria [Scintilla](https://www.scintilla.org/) e fornisce funzionalità
specializzate per il codice sorgente nuBASIC.

**Funzionalità principali:**

- **Evidenziazione della sintassi** — le parole chiave, i letterali, i commenti e gli identificatori di
  nuBASIC sono resi in colori distinti, rendendo immediatamente visibile la struttura del programma.
- **Compressione del codice (code folding)** — le sezioni del codice possono essere compresse in una
  singola riga di riepilogo e successivamente espanse di nuovo; i punti di fold sono determinati dai
  livelli di indentazione.
- **Completamento automatico** — premendo `F12` si apre un elenco a comparsa delle parole chiave
  corrispondenti; premere `Tab` per inserire l'elemento evidenziato.
- **Segnalibri** — qualsiasi riga può essere contrassegnata con `Ctrl+F7`; la navigazione tra i
  segnalibri li percorre in avanti (`Ctrl+F8`) o all'indietro (`Ctrl+F9`).
- **Trova e Sostituisci** — ricerca full-text con sensibilità facoltativa alle maiuscole/minuscole,
  corrispondenza di parole intere e sostituzione con espressioni regolari.
- **Guida contestuale** — selezionare una parola chiave e premere `F1` per visualizzare la voce della
  guida integrata per quella parola chiave direttamente dall'interprete; premere `Ctrl+F1` per aprire la
  guida online in un browser.
- **Debugger integrato** — breakpoint, esecuzione passo per passo, valutazione di espressioni
  (data-tip) e tracciamento del contatore di programma sono tutti accessibili senza lasciare l'editor.
- **Indicatore di riga e colonna** — la barra degli strumenti mostra in modo permanente la posizione
  del cursore.
- **Zoom** — `F3` ingrandisce; `Ctrl+F3` riduce.

---

### 7.2 Comandi da Tastiera

I tasti di spostamento dell'editor (`Freccia`, `Page Up/Down`, `Home`, `End`) estendono una selezione
continua quando si tiene premuto `Shift`, e estendono una selezione rettangolare quando si tengono
premuti sia `Shift` che `Alt`. Le scorciatoie da tastiera per i comandi di menu sono elencate nei menu
stessi. I comandi aggiuntivi senza equivalente di menu sono:

| Azione | Tasto |
|---|---|
| Indenta blocco | `Tab` |
| Dedenta blocco | `Shift+Tab` |
| Elimina fino all'inizio della parola | `Ctrl+Backspace` |
| Elimina fino alla fine della parola | `Ctrl+Delete` |
| Elimina fino all'inizio della riga | `Ctrl+Shift+Backspace` |
| Elimina fino alla fine della riga | `Ctrl+Shift+Delete` |
| Vai all'inizio del documento | `Ctrl+Home` |
| Estendi la selezione all'inizio del documento | `Ctrl+Shift+Home` |
| Vai all'inizio della riga visualizzata | `Alt+Home` |
| Vai alla fine del documento | `Ctrl+End` |
| Estendi la selezione alla fine del documento | `Ctrl+Shift+End` |
| Vai alla fine della riga visualizzata | `Alt+End` |
| Scorra su | `Ctrl+Up` |
| Scorra giù | `Ctrl+Down` |
| Taglia riga | `Ctrl+L` |
| Elimina riga | `Ctrl+Shift+L` |
| Duplica selezione | `Ctrl+D` |
| Paragrafo precedente (Shift estende la selezione) | `Ctrl+[` |
| Paragrafo successivo (Shift estende la selezione) | `Ctrl+]` |
| Parola precedente (Shift estende la selezione) | `Ctrl+Left` |
| Parola successiva (Shift estende la selezione) | `Ctrl+Right` |
| Parte di parola precedente (Shift estende la selezione) | `Ctrl+/` |
| Parte di parola successiva (Shift estende la selezione) | `Ctrl+\` |
| Selezione rettangolare a blocchi | `Alt+Shift+Movement` |
| Estendi la selezione rettangolare all'inizio della riga | `Alt+Shift+Home` |
| Estendi la selezione rettangolare alla fine della riga | `Alt+Shift+End` |
| Vai alla riga… | `F2` |
| Ingrandisci | `F3` |
| Riduci | `Ctrl+F3` |

---

### 7.3 Compressione del Codice

Il codice sorgente di un programma ha una gerarchia naturale di sezioni e sottosezioni. La funzione di
fold consente di nascondere qualsiasi sezione, sostituendola con un'unica riga di panoramica, e di
mostrarla di nuovo in seguito. I punti di fold sono determinati dall'indentazione.

I marcatori dei punti di fold nel margine di fold si comportano come segue:

- **Clic** — espande o comprime quel fold senza influire sui suoi figli. I figli che erano stati
  compressi singolarmente rimangono tali quando il genitore viene espanso.
- **Ctrl+Clic** — attiva/disattiva il fold e applica la stessa operazione ricorsivamente a tutti i figli.
- **Shift+Clic** — espande tutti i fold figli senza attivare/disattivare il fold su cui si fa clic.
- **Ctrl+Shift+Clic in un punto qualsiasi del margine di fold** — espande o contrae tutti i fold di
  primo livello contemporaneamente. La voce di menu **Visualizza → Attiva/disattiva tutti i fold** fa
  lo stesso.

> **Suggerimento:** Per aprire un blocco grande con tutti i nipoti ancora compressi, comprimerlo prima
> con `Ctrl+Clic`, poi espanderlo con un semplice clic. I fold figli saranno visibili ma ancora
> compressi; aprirli singolarmente secondo necessità.

---

### 7.4 Trova e Sostituisci

**Trova (`Ctrl+F` o Cerca → Trova…)**

Apre la finestra di dialogo Trova. Immettere il testo da cercare nella casella *Trova cosa* (la
selezione corrente precompila questa casella). Opzioni:

- **Direzione** — cerca verso l'alto verso l'inizio del documento o verso il basso verso la fine.
- **Solo parola intera** — trova corrispondenza solo quando la stringa è delimitata da spazi, tabulazioni,
  punteggiatura o fine riga.
- **Maiuscole/minuscole** — ricerca con distinzione tra maiuscole e minuscole; se deselezionato la
  ricerca non distingue maiuscole e minuscole.
- **Trova successivo** — si sposta alla prossima occorrenza.

**Trova e Sostituisci (`Ctrl+R` o Cerca → Trova e Sostituisci…)**

Aggiunge una casella *Sostituisci con* a quanto sopra, più:

- **Sostituisci** — sostituisce la corrispondenza corrente e avanza alla successiva.
- **Sostituisci tutto** — sostituisce ogni occorrenza in un'unica operazione.
- La sostituzione con espressioni regolari è supportata.

---

### 7.5 Segnalibri

Un segnalibro è un marcatore permanente associato a una riga sorgente. Le righe con segnalibro sono
indicate da un cerchio nel margine sinistro.

| Comando di menu | Scorciatoia | Descrizione |
|---|---|---|
| Aggiungi/Rimuovi marcatore | `Ctrl+F7` | Attiva/disattiva un segnalibro sulla riga corrente |
| Trova marcatore successivo | `Ctrl+F8` | Salta al segnalibro successivo |
| Trova marcatore precedente | `Ctrl+F9` | Salta al segnalibro precedente |
| Rimuovi tutti i marcatori | — | Cancella ogni segnalibro nel documento corrente |

Tutti i comandi dei segnalibri sono disponibili anche nel menu **Cerca**.

---

### 7.6 Completamento Automatico

Premendo `F12` si apre un elenco a comparsa popolato con le parole chiave di nuBASIC che
corrispondono ai caratteri digitati finora. Navigare nell'elenco con i tasti freccia e premere `Tab`
per inserire la parola chiave selezionata nella posizione del cursore.

---

### 7.7 Barra degli Strumenti

La barra degli strumenti fornisce accesso con un solo clic alle operazioni più frequenti su file,
debug e ricerca. Mostra inoltre il numero di riga e di colonna correnti del cursore, e un indicatore
quando il documento ha modifiche non salvate.

| Pulsante | Voce di menu | Scorciatoia |
|---|---|---|
| Nuovo | File → Nuovo | `Ctrl+N` |
| Apri | File → Apri… | `Ctrl+O` |
| Salva | File → Salva | `Ctrl+S` |
| Debug | Debug → Avvia debug | `F5` |
| Interrompi | Debug → Arresta debug | `Esc` |
| Breakpoint | Debug → Attiva/disattiva breakpoint | `F9` |
| Compila | Debug → Compila programma | `Ctrl+B` |
| Valuta | Debug → Valuta selezione | — |
| Step Into | Debug → Step Into | `F11` |
| Step Over | Debug → Step Over | `F10` |
| Step Out | Debug → Step Out | `Shift+F11` |
| Cont | Debug → Continua debug | `F8` |
| Trova | Cerca → Trova… | `Ctrl+F` |
| Con Top | Debug → Console in primo piano | — |
| Ide Top | Debug → IDE in primo piano | — |

---

### 7.8 Guida Contestuale e Guida Online

**Guida contestuale (`F1`)**

Selezionare una parola chiave nell'editor (o posizionare il cursore su di essa) e premere `F1`. L'IDE
interroga il database della guida integrata dell'interprete e visualizza la descrizione completa di
quella parola chiave nel pannello di output — le stesse informazioni restituite da `Help <keyword>` nel
REPL, senza lasciare l'editor.

**Guida online (`Ctrl+F1`)**

Con una parola chiave selezionata, premendo `Ctrl+F1` si costruisce una query di ricerca per la parola
chiave e la si apre nel browser web predefinito, visualizzando l'argomento della guida online pertinente.

---

### 7.9 Debugger Integrato

Il debugger integrato lavora in stretto coordinamento con l'editor con evidenziazione della sintassi. I
breakpoint appaiono come cerchi rossi nel margine sinistro; la freccia del contatore di programma
indica la prossima riga da eseguire; i risultati delle espressioni appaiono come annotazioni inline
direttamente nel sorgente.

#### Menu Debug

| Comando | Scorciatoia | Descrizione |
|---|---|---|
| Compila programma | `Ctrl+B` | Compila i metadati del programma utilizzati per tutte le esecuzioni successive. Viene eseguito automaticamente la prima volta che il programma viene avviato e ogni volta che il sorgente è stato modificato. |
| Avvia debug | `F5` | Avvia il programma con il debugger collegato. L'esecuzione continua fino a quando non viene raggiunto un breakpoint, viene premuto `Ctrl+C`, si verifica un errore o il programma termina normalmente. |
| Arresta debug | `Esc` | Termina immediatamente il programma in esecuzione. |
| Continua debug | `F8` | Riprende l'esecuzione dal breakpoint corrente fino al prossimo breakpoint o alla fine del programma. |
| Step Into | `F11` | Esegue la riga corrente. Se la riga chiama un `Sub` o una `Function`, l'esecuzione scende dentro la routine chiamata e si ferma alla sua prima istruzione. |
| Step Over | `F10` | Esegue la riga corrente come unita' singola. Eventuali chiamate dentro la riga vengono eseguite per intero; il debugger si ferma alla riga successiva del *chiamante*. |
| Step Out | `Shift+F11` | Riprende l'esecuzione fino a quando la `Sub` o `Function` corrente non termina, e si ferma alla riga successiva del chiamante. |
| Esegui fino al cursore | `Ctrl+F10` | Riprende l'esecuzione fino alla riga sotto il cursore, poi si ferma. Equivalente a un breakpoint temporaneo "una tantum". |
| Pausa / Break | `Ctrl+Pause` | Interrompe un programma in esecuzione alla prossima istruzione su cui si puo' fermare ed entra nel debugger. Utile per i loop senza breakpoint o per i programmi avviati con Continua. |
| Valuta selezione (Data-Tip) | — | Valuta la variabile o l'espressione selezionata nel contesto del programma in pausa e visualizza il risultato come annotazione inline accanto alla selezione. Utile per ispezionare espressioni complesse senza aggiungere istruzioni `Print` aggiuntive. |
| Avvia senza debug | `Ctrl+F5` | Esegue il programma senza il debugger collegato. Premere `Ctrl+C` durante l'esecuzione per interrompere e tornare al prompt del CLI (vedi capitolo 3). |
| Attiva/disattiva breakpoint | `F9` | Aggiunge o rimuove un breakpoint sulla riga corrente. I breakpoint attivi sono mostrati come un cerchio rosso nel margine. |
| Elimina tutti i breakpoint | — | Rimuove ogni breakpoint nel documento corrente. |
| Vai al contatore di programma | — | Scorre l'editor fino alla riga che verrà eseguita successivamente quando l'esecuzione viene ripresa. |
| Vai alla procedura | — | Apre un sottomenu dinamico che elenca ogni funzione e subroutine nel programma. Selezionando una voce il cursore viene spostato al punto di ingresso di quella routine. L'elenco viene rigenerato ogni volta che il programma viene compilato. |
| Console in primo piano | — | Mantiene la finestra della console nuBASIC sopra tutte le altre finestre, anche quando perde il focus — utile quando si esegue un programma che produce output grafico accanto all'editor. |
| IDE in primo piano | — | Ripristina il normale ordine z per la console e porta la finestra IDE in primo piano. |

#### Pannello di Output

Nella parte inferiore dell'IDE si trova un pannello a schede con due schede:

- **Output** — visualizza i messaggi dell'interprete, i risultati della compilazione e i risultati di
  valutazione dai Data-Tip. I messaggi sono codificati a colori: output normale su sfondo bianco,
  avvertenze su sfondo giallo, errori su sfondo rosso.
- **Console** — incorpora la finestra della console grafica di nuBASIC. Tutto l'output di `Print`, i
  disegni grafici e l'input da mouse/tastiera del programma in esecuzione appaiono qui.

Il pannello può essere ridimensionato, nascosto o staccato come finestra mobile tramite il menu
**Impostazioni**.

---

### 7.10 Estensione VS Code

nuBASIC fornisce un'estensione VS Code che offre evidenziazione della
sintassi, un pulsante Run e un debug adapter compatibile con la UI di
debug standard di VS Code (breakpoint, step in/over/out, watch, call
stack, variabili).

#### Installazione

Il componente opzionale **VSCodeExtension** dell'MSI di Windows
installa l'estensione automaticamente nell'istanza locale di VS Code.
Per installarla manualmente:

```
code --install-extension <installazione-nubasic>/bin/nubasic-latest.vsix
```

Su Linux / macOS lo stesso `.vsix` e' distribuito sotto
`<install-prefix>/bin/`; si installa con lo stesso comando.

#### Esecuzione e debug

Apri un file `.bas` (o un file di progetto `.nbp`) in VS Code, poi premi
**F5**. L'estensione crea un `launch.json` di tipo `nubasic` di default
al primo utilizzo. Configurazioni disponibili:

| `request` | Effetto |
|-----------|---------|
| `launch` | Esegue il file corrente con il debugger collegato |
| `launch` + `noDebug: true` | Esegue senza fermarsi sui breakpoint (equivale a **Run → Run Without Debugging**) |
| `launch` + `mode: "graphics"` | Usa la modalita' ibrida `--graphics-window` per i programmi grafici: il debugger e stdout restano nella sessione VS Code mentre la grafica viene disegnata in una finestra GDI separata |

L'estensione individua l'interprete in questo ordine:

1. Il setting `nubasic.executablePath` (se impostato).
2. La variabile d'ambiente `NUBASIC_HOME`, poi `%PATH%`.
3. La chiave di registro `HKCU\Software\nuBASIC\InstallDir` (Windows).

Per le sessioni di debug viene sempre lanciato `nubasicdebug` (o
`nubasicdebug.exe` su Windows) e `nubasic` per le sessioni Run, in modo
che VS Code mantenga il controllo di stdio mentre l'eventuale finestra
GDI sia solo un target di rendering.

---

### 7.11 Progetti multi-sorgente

I programmi piu' lunghi di un singolo file possono essere organizzati
come *progetto*. Un progetto nuBASIC e' descritto da un piccolo
manifest testuale con estensione `.nbp` che registra tre cose: lo
script di entry, la modalita' di sintassi e un nome visualizzato. I
file inclusi vengono comunque caricati dalle direttive `Include` /
`#Include` all'interno dello script di entry — il `.nbp` indica solo
all'IDE quale e' il file di entry.

#### Formato del file di progetto

```text
name = MioProgramma
syntax = modern
entry = src/main.bas
```

| Chiave | Significato |
|--------|-------------|
| `name` | Nome visualizzato nella barra del titolo dell'IDE e nei progetti recenti |
| `syntax` | `legacy` o `modern` — modalita' `Syntax` iniziale per il file di entry |
| `entry` | Percorso del file `.bas` di entry, risolto rispetto alla directory contenente il `.nbp` |

Le righe che iniziano con `'` o `#` vengono ignorate, quindi il file
puo' contenere commenti. Le chiavi sconosciute sono tollerate ma
ignorate — gli IDE piu' vecchi continueranno a funzionare anche se
in futuro verranno aggiunti nuovi campi.

#### Lavorare con i progetti dall'IDE

L'IDE Windows ha comandi dedicati per l'editing project-aware:

| Comando di menu | Descrizione |
|-----------------|-------------|
| **File → Apri progetto…** | Seleziona un file `.nbp`. L'IDE carica lo script di entry, imposta la modalita' di sintassi dichiarata e aggiorna la barra del titolo con il nome del progetto. |
| **File → Chiudi progetto** | Stacca il progetto corrente; l'editor torna in modalita' single-file. |
| **File → Progetti recenti** | Elenca i file `.nbp` aperti di recente per riaprirli rapidamente. |

Una volta aperto un progetto:

- **Build / Avvia debug** usa lo script di entry come punto d'ingresso
  del programma. Il debugger attraversa i file `Include` in modo
  trasparente; i breakpoint in ognuno di essi vengono rispettati.
- **Step Into** attraversa il confine di un `Include` allo stesso
  modo in cui attraversa un confine `Sub` / `Function`; il pannello
  call stack mostra nome file e numero di riga per ogni frame.
- **Trova / Sostituisci** opera ancora solo sul documento attivo —
  *non* e' ancora una ricerca a livello di progetto — ma segnalibri e
  recent-jump sono memorizzati per file attraverso le sessioni di
  progetto.

#### Lavorare con i progetti dalla CLI

Si puo' passare il percorso del `.nbp` da riga di comando per
eseguire l'intero progetto come unita':

```
nubasic -e MioProgramma.nbp
nubasicdebug MioProgramma.nbp
```

L'interprete risolve lo script di entry, applica la modalita' di
sintassi dichiarata ed esegue il programma esattamente come se il
progetto fosse stato aperto dall'IDE.

Il repository include un esempio in `tests/test_multisource.nbp`.

---

## 8. Storia ed Evoluzione di nuBASIC

### Origini (marzo 2014)

nuBASIC è stato creato da Antonino Calderone e pubblicato per la prima volta nel marzo 2014, a
partire dalla versione 0.1. L'implementazione iniziale ha stabilito il nucleo di quello che
sarebbe diventato un linguaggio completo: un interprete di espressioni, il supporto alle
variabili e un insieme di funzioni matematiche fondamentali
(`Sin`, `Cos`, `ASin`, `ACos`, e le altre funzioni trigonometriche e trascendenti standard).

Nello stesso mese comparvero le principali istruzioni BASIC: `Let`, `Print`, `Input`, `GoTo`,
`If/Then/Else`, `For/To/Step`, e i comandi interattivi essenziali `Run`, `List`, `New`,
`Clr`. Una console I/O fu implementata sia per Windows che per Linux, rendendo nuBASIC
immediatamente utilizzabile su entrambe le piattaforme. I pacchetti `.deb` per Ubuntu/Debian e
i pacchetti RPM per Fedora e openSUSE seguirono poco dopo.

### Crescita del linguaggio (aprile – giugno 2014)

La primavera del 2014 portò un ritmo notevole di espansione. Ogni release aggiungeva
funzionalità che trasformarono nuBASIC da un interprete giocattolo in un vero strumento di
programmazione:

- **Operazioni su file** — `Open`, `Close`, `Input#`, `Print#`, `FOpen`, `Seek`, `FTell`, `FSize`,
  e array di byte (suffisso `@`), che permettono ai programmi di leggere e scrivere dati
  persistenti (v0.15)
- **Subroutine e funzioni** — `Sub`/`End Sub`, `Function`/`End Function`, `Exit Sub`,
  `Exit Function`, con stack di chiamata e scope di variabili locali (v0.18, v0.19)
- **Cicli strutturati** — `While`/`Wend` e `Do`/`Loop While`, riducendo la dipendenza da `GoTo` (v0.16)
- **Supporto alle etichette** — `GoTo` e `GoSub` potevano ora puntare a etichette con nome
  oltre che a numeri di riga, rendendo il codice molto più leggibile (v0.14)
- **Breakpoint e tracciatura** — breakpoint condizionali (`Break If`), tracciatura del programma
  (`TrOn`/`TrOff`), ed esecuzione passo-passo (`StOn`/`StOff`) per il debug (v0.13–v0.20)
- **Prima grafica** — `Line`, `Rect`, `FillRect`, `Ellipse`, `FillEllipse` su Windows (GDI)
  e Linux (X11), poi `TextOut` e `SetPixel` (v0.7–v0.9)
- **Supporto al mouse** — `GetMouseX()`, `GetMouseY()`, `GetMouseBtn()` (v1.06)
- **Funzione `Eval`** — valuta un'espressione nuBASIC da una stringa a runtime (v1.10)
- Primi esempi di giochi: `games/breakout.bas` (grafica ASCII art), `games/breakout2.bas` (grafica GDI),
  `minehunter.bas` (il clone di Mine Hunter)

In parallelo, il nuBasicEditor — un IDE per Windows con evidenziazione della sintassi,
autocompletamento, guida contestuale (F1), segnalibri e una barra degli strumenti — crebbe
dalla sua prima release (v0.2) fino alla v1.11.

### Espansione del sistema dei tipi (nov 2014 – nov 2015)

Nel corso del 2015, nuBASIC ricevette una serie di release che approfondirono il sistema dei
tipi e lo avvicinarono ai moderni dialetti BASIC strutturati:

- **Boolean** (postfisso `#`) e **Long64** (postfisso `&`) come tipi numerici, insieme alle
  costanti letterali `True` e `False` (v1.24)
- **Operatori unari** — inclusi il prefisso in stile C `++` e `--` (v1.24)
- **Sequenze di escape** — `\n`, `\r`, `\t`, `\a`, e altre all'interno dei letterali stringa (v1.24)
- **Istruzione `Const`** — costanti con nome e immutabili, con tipo esplicito opzionale (v1.22)
- **`Elif` / `ElseIf`** — condizionali multi-ramo più chiari senza annidamento (v1.25–v1.26)
- **Istruzione `Stop`** — interrompe l'esecuzione e torna al REPL, riprendibile con `Cont` (v1.51)
- **Dichiarazioni di tipo esplicito** — `Dim x As Integer`, `Dim s As String`, eliminando
  l'ambiguità per programmi complessi (v1.40)
- **`Struct`** — tipi compositi definiti dall'utente con campi con nome, tipi di campo
  espliciti e pieno supporto all'annidamento (v1.40)
- **Sintassi estesa delle funzioni** — valori di ritorno tipizzati, funzioni che restituiscono
  oggetti struct e funzioni che restituiscono array per valore (v1.42, v1.48)

Comparvero nuovi programmi grafici di esempio: `clock.bas` (un orologio analogico),
`plane.bas` (un'animazione di aereo in volo), e `GetPixel` fu aggiunto per consentire la
lettura dei colori dei pixel.

### Tabelle hash, formattazione in virgola mobile e altro (gen 2017)

La versione 1.47 aggiunse diverse funzionalità di convenienza che colmarono lacune nella
libreria standard:

- **Tabelle hash** — `HSet`, `HGet`, `HCnt`, `HChk`, `HDel` — che forniscono un contenitore
  associativo integrato senza bisogno di implementarne uno da zero
- **Tipo `Any`** — una variabile che assume automaticamente il tipo di qualsiasi valore le
  venga assegnato, abilitando contenitori generici e l'API delle tabelle hash
- **`StrP` / `StrP$`** — formattazione in virgola mobile con un numero specificato di cifre
  significative, colmando il divario tra `Str$` e le stringhe di formato manuali
- **`GetAppPath()`** — il percorso dell'eseguibile nuBASIC, utile per caricare risorse
  relative alla posizione di installazione
- **`Quit()`** — esce dall'interprete dall'interno di un programma

### IDE per Linux, licenza MIT (lug 2017)

La versione 1.48 fu un traguardo strategico. L'IDE fu **portato su Linux/GTK+**, rendendo
l'ambiente di sviluppo completo disponibile sia su Windows che su Linux per la prima volta. La
licenza fu cambiata in **MIT**, rimuovendo tutte le precedenti restrizioni d'uso. La release
aggiunse anche lo zoom in/out nell'editor, `SetTopmost`, aggiornò stb\_image alla 2.15 e
Scintilla alla 3.74.

### READ / DATA / RESTORE (lug 2019)

La versione 1.50 aggiunse le classiche istruzioni `Read`, `Data` e `Restore`. Queste
consentono ai programmi di incorporare tabelle di costanti strutturate — palette, mappe,
configurazioni, dati sprite — direttamente nel codice sorgente, di leggerle sequenzialmente
con `Read` e di rileggerle da qualsiasi posizione con `Restore`. La funzionalità era stata
una parte standard del BASIC fin dagli anni '70 e la sua aggiunta colmò un'importante lacuna
di compatibilità.

### Miglioramenti alla qualità d'uso (set 2019)

La versione 1.51 affrontò diversi problemi di correttezza:

- Corretto un errore di regressione nel parser in cui l'operatore meno unario poteva generare
  un errore di sintassi spurio
- Aggiunto `Stop` per compatibilità con altri interpreti BASIC
- Esteso `If` per supportare un `GoTo` implicito dopo `Then` ed `Else` (ad es.
  `If x Then myLabel` senza la parola chiave `GoTo`)
- Corretto il disallineamento della posizione del mouse nelle configurazioni di display ad alta
  risoluzione/scalate su Windows

### Supporto Unicode (feb 2022)

La versione 1.52 ha corretto un bug per cui `Const` non funzionava correttamente con i valori
stringa. Più significativamente, ha aggiunto il supporto per le **sequenze di escape Unicode**
nei letterali stringa: anteponendo `$u` a un'istruzione `Print`, l'interprete riconosce le
sequenze `\u0000`–`\uFFFF` e restituisce i corrispondenti caratteri Unicode. Il modo in cui
appaiono dipende dalla configurazione del font della console.

### Console Windows personalizzata, UTF-8, programma di installazione MSI (marzo 2026)

La versione 1.60 fu la release infrastrutturale più grande dalla versione originale:

- Una **nuovissima finestra di console basata su GDI** ha sostituito la console Windows
  standard. La nuova console esegue il rendering di testo e grafica in modo nativo tramite
  GDI, funziona correttamente sia in modalità autonoma che incorporata nell'IDE, evita le
  limitazioni della Windows Console API (che non supporta grafica a pixel arbitrari) e
  renderizza i font proporzionali in modo pulito. La finestra può essere spostata,
  ridimensionata e disegnata dal codice BASIC con qualsiasi granularità.
- **Supporto completo UTF-8** end-to-end: i caratteri al di fuori di ASCII — Latino Esteso,
  Cirillico, Greco, CJK, emoji — sono ora preservati attraverso il tokenizer, il runtime
  dell'interprete e vengono renderizzati dalla console GDI. Due bug di eliminazione silenziosa
  furono corretti: uno nel percorso di caricamento file della CLI (`nu_interpreter.cc`) e uno
  nel percorso di ricostruzione del codice dell'editor IDE (`nuBasicEditor.cc`). Il nuovo
  esempio `hello_world_languages.bas` dimostra l'output `Print` multilingue in 20 lingue in
  un singolo programma.
- Fu introdotto un **programma di installazione MSI**, che consente la corretta integrazione
  con Installazione applicazioni di Windows, i collegamenti sul desktop e la disinstallazione
  pulita.
- Scintilla fu aggiornata alla sua ultima versione.

### nuBASIC 2.0 — Nuove funzionalità del linguaggio (2026)

La versione 2.0 è una release maggiore che rimuove API deprecate da tempo e aggiunge nuove
funzionalità significative. Le caratteristiche di punta (tutte documentate in dettaglio in
precedenza in questa guida):

- **Programmazione orientata agli oggetti** — `Class` con ereditarietà singola (`Inherits`),
  dispatch virtuale (`Overridable`/`Overrides`/`MyBase`), controllo degli accessi a tre livelli,
  costruttori (`Sub New`) e distruttori RAII (`Sub Delete`), e metodi `Static` di classe
  ([sezione 4.5](#45-classi-e-oggetti)).
- **Punto di ingresso `main()`** ([4.6](#46-punto-di-ingresso-main)), **moduli con namespace**
  (`Syntax Modern`, `Using`, `math::sin`) ([4.7](#47-moduli-con-namespace)) e **parametri array
  a dimensione aperta** (`param() As Type`).
- **Dispatch `Select Case`**, con liste di valori, intervalli `To`, guardie di confronto `Is` e
  case stringa.
- **Chiamate a librerie native** — `Declare Function … Lib` per chiamare DLL Windows, `.so`
  Linux e `.dylib` macOS tramite libffi, con continuazione di riga `_` per dichiarazioni
  leggibili ([4.14](#414-chiamate-a-librerie-native)).
- **Modello di debugger moderno** — comandi riutilizzabili Continue / Step Into / Step Over /
  Step Out / Pause / Run-to-Cursor condivisi dall'IDE nativo, dalla CLI di debug e
  dall'adattatore VS Code, oltre al debug consapevole della grafica tramite una finestra GDI
  separata.
- **Ottimizzazione del runtime** — `variant_t` memorizza gli scalari inline, incapsula
  (box) i metadati di struct/oggetti e usa il copy-on-write per i payload delle struct; letterali
  esadecimali estesi `&hFF` / `0xFF`.
- **Integrazione WinRayCast (Windows)** — un motore di raycasting integrato per il rendering
  pseudo-3D in prima persona, in stile *Wolfenstein 3D*, pilotato dal BASIC attraverso il modulo
  `Ray…` ([sezione 5.5](#55-pseudo-3d-con-winraycast)). È distinto dal precedente esempio di
  raycaster software in puro BASIC `raycast3d.bas`; l'esempio incluso
  `examples/raycast/eclipse_protocol.bas` è una demo completa e giocabile.
- **API deprecate rimosse** — le funzioni scalari di data/ora `Sys*` e `GetMouseX` /
  `GetMouseY` / `GetMouseBtn` (deprecate nella v1.62) sono state eliminate; usa `GetDateTime()` e
  `GetMouse()`.

### Rendering grafico senza sfarfallio (aprile 2026, v1.61)

La versione 1.61 ha affrontato l'ultimo grande problema di qualità visiva: lo sfarfallio dello
schermo per-primitiva nei programmi animati.

Prima di questa release, ogni chiamata grafica — `Line`, `FillRect`, `TextOut`, e tutte le
altre — innescava un blit immediato del back buffer sullo schermo visibile. Un frame composto
da dieci comandi di disegno produceva dieci stati intermedi visibili. Giochi, orologi,
simulazioni e le demo frattali presentavano tutti questo artefatto in varia misura.

Due correzioni complementari furono implementate a livello C++:

1. **Correzione del compositing della pittura** — I caratteri di testo e il cursore di testo
   vengono ora compositati sul back buffer fuori schermo *prima* della singola copia sullo
   schermo, eliminando lo sfarfallio di doppia pittura che si verificava durante i ridisegni
   della finestra.

2. **Istruzioni `ScreenLock` / `ScreenUnlock` / `Refresh`** — Un nuovo flag booleano atomico
   (`_render_locked`) in `ConsoleWindow` sopprime la chiamata automatica a `refresh()` dentro
   `release_offscreen_dc()`. Le tre nuove istruzioni BASIC espongono questo meccanismo
   direttamente ai programmi. Quattordici programmi grafici di esempio distribuiti con nuBASIC
   furono aggiornati per utilizzare il pattern: `breakout2.bas`, `breakout3.bas`,
   `pingpong.bas`, `nutetris.bas`, `nutetris3d.bas`, `raycast3d.bas`, `plane.bas`,
   `clock.bas`, `cartoon.bas`, `minesHunter.bas`, `tictactoe.bas`, `line.bas`, `rosetta.bas`
   e `calc.bas`.

Le voci della guida integrata per tutte e tre le nuove istruzioni furono aggiunte a
`nu_builtin_help.cc` in modo che `Help ScreenLock`, `Help ScreenUnlock` e `Help Refresh`
restituiscano la documentazione completa dal REPL.

## 9. Come è fatto nuBASIC: Architettura Interna dell'Interprete

Scrivere un interprete BASIC in C++ moderno è un esercizio sorprendentemente piacevole. Il
BASIC stesso porta con sé una forte nostalgia per la prima era dell'informatica personale —
macchine come il Commodore 64 erano fornite di un interprete BASIC in ROM, e per un'intera
generazione quell'interprete *era* il computer. Ricreare quell'esperienza da zero, con gli
strumenti del C++ contemporaneo, cattura sia la semplicità dell'originale che la correttezza
che deriva da sistemi di tipi migliori e dall'infrastruttura della libreria standard.

Questo capitolo descrive l'architettura interna dell'interprete nuBASIC. È rivolto agli
sviluppatori che vogliono capire come funziona l'interprete, contribuire alla codebase o
utilizzarla come base per i propri esperimenti con i linguaggi. Tutte le classi e i file
menzionati sono nel namespace `nu`; i file header si trovano in `include/` e le implementazioni
in `lib/`.

---

### 9.1 Componenti Principali

L'interprete segue la pipeline classica di un elaboratore di linguaggi: il testo sorgente
entra, i token escono, i token diventano un albero sintattico astratto e l'albero viene
eseguito in un contesto di runtime. I componenti seguenti implementano questa pipeline. Non
sempre corrispondono uno a uno a una singola classe C++ — alcuni sono raggruppamenti concettuali
— ma ognuno ha una collocazione precisa nell'albero dei sorgenti.

| Componente | Classe / Tipo | File principale |
|---|---|---|
| **Tokenizer** | `tokenizer_t` (estende `expr_tknzr_t`) | `include/nu_tokenizer.h` |
| **Token** | `token_t` | `include/nu_token.h` |
| **Lista di token** | `token_list_t` | `include/nu_token_list.h` |
| **Parser di espressioni** | `expr_parser_t` | `include/nu_expr_parser.h` |
| **Nodo base dell'espressione** | `expr_any_t` | `include/nu_expr_any.h` |
| **Parser di istruzioni** | `stmt_parser_t` | `include/nu_stmt_parser.h` |
| **Base dell'istruzione** | `stmt_t` | `include/nu_stmt.h` |
| **Contesto di programma statico** | `prog_ctx_t` | `include/nu_prog_ctx.h` |
| **Contesto di programma a runtime** | `rt_prog_ctx_t` (estende `prog_ctx_t`) | `include/nu_rt_prog_ctx.h` |
| **Mappa delle righe del programma** | `prog_line_t` | `include/nu_program.h` |
| **Interprete** | `interpreter_t` | `include/nu_interpreter.h` |
| **Valore variant** | `variant_t` | `include/nu_variant.h` |
| **Guida integrata** | `builtin_help_t` | `include/nu_builtin_help.h` |

**Tokenizer** — scompone una riga sorgente in una sequenza piatta di oggetti `token_t` tipizzati.

**Parser di espressioni** — legge una sequenza di token che rappresenta un'espressione come
`2 + 4 * 17` e restituisce un `expr_any_t::handle_t` (un `std::shared_ptr<expr_any_t>`) che
può essere valutato su richiesta chiamando il suo metodo `eval()`.

**Valutatore di espressioni** — il metodo virtuale `eval()` su ciascuna sottoclasse di
`expr_any_t`. Chiamarlo con un `rt_prog_ctx_t` produce un risultato `variant_t`.

**Parser di istruzioni** (`stmt_parser_t`) — itera sulla lista di token di una riga sorgente
completa e costruisce un albero di oggetti istruzione eseguibili. Chiama il Parser di
espressioni ogni volta che incontra un'espressione.

**Esecutore di istruzioni** — il metodo virtuale `run()` su ciascuna sottoclasse di `stmt_t`.

**Contesto di programma statico** (`prog_ctx_t`) — accumulato durante la fase di build. Contiene
i prototipi delle procedure, le definizioni delle strutture, le tabelle delle etichette e i
metadati necessari per risolvere i costrutti multi-riga come `For`/`Next`, `While`/`Wend`,
`If`/`ElseIf`/`EndIf` e le definizioni di procedure annidate.

**Contesto di programma a runtime** (`rt_prog_ctx_t`) — estende `prog_ctx_t` con tutto lo stato
che esiste solo mentre un programma è in esecuzione: i valori delle variabili, lo stack delle
chiamate di procedura, i dati di runtime del ciclo `For`, la tabella dei descrittori di file,
il contatore di programma e i registri dei valori di ritorno per le funzioni.

**Interprete** (`interpreter_t`) — possiede le righe sorgente, il programma analizzato, i
contesti statico e di runtime e il parser di istruzioni. Implementa il ciclo dei comandi CLI,
la fase `rebuild()`, il ciclo `run()` e il debugger.

**Libreria di funzioni integrate** — le singole funzioni integrate (`Sin`, `Len`, `Mid$`, …)
sono implementate come sottoclassi di `expr_any_t` il cui metodo `eval()` esegue il calcolo.

**Oggetti delle istruzioni del linguaggio** — ogni istruzione BASIC (`Print`, `For`…`Next`,
`If`…`EndIf`, `Sub`, `Open`, …) è una sottoclasse di `stmt_t` con un override di
`run(rt_prog_ctx_t&)`.

**Gestori di errori sintattici e di runtime** — classi di eccezione C++ utilizzate in tutta la
pipeline per segnalare gli errori del tokenizer, del parser e di runtime al livello
dell'interprete.

**Guida integrata** — la classe `builtin_help_t` implementa i comandi CLI `Help` e `Apropos`
mantenendo una tabella parola chiave-descrizione compilata nell'interprete.

---

### 9.2 Un Interprete Orientato alle Righe

Il BASIC è un linguaggio orientato alle righe: la posizione di un'interruzione di riga nel
sorgente è sintatticamente significativa in un modo che non lo è in C++ o Python. nuBASIC
preserva questa proprietà lungo l'intera pipeline.

Il testo sorgente è memorizzato in due mappe parallele dentro `interpreter_t`:

- `_source_line` — associa un `line_num_t` alla stringa sorgente grezza, esattamente come
  digitata.
- `_prog_line` — un oggetto `prog_line_t` (una `std::map<line_num_t, std::pair<stmt_t::handle_t, dbginfo_t>>`)
  che associa gli stessi numeri di riga agli oggetti istruzione compilati più le informazioni
  di debug.

Durante la fase di build (`rebuild()`) ogni riga sorgente viene passata a
`stmt_parser_t::compile_line()`, che la trasforma in token e chiama `parse_block()`. Il
risultato — un puntatore condiviso a un oggetto istruzione block — viene memorizzato in
`_prog_line` sotto il numero di quella riga.

Il contesto di programma statico (`prog_ctx_t`) viene costruito nello stesso momento. Unisce
i costrutti multi-riga: quando il parser di istruzioni incontra `For` registra un riferimento
in avanti in `for_loop_metadata`; quando incontra poi il `Next` corrispondente risolve quel
riferimento. Lo stesso meccanismo si applica a `While`/`Wend`, `Do`/`Loop While`,
`If`/`ElseIf`/`EndIf`, `Sub`/`Function`/`End Sub` e `Type`/`End Type`.

Al momento dell'esecuzione, `run()` itera su `_prog_line` in ordine di chiave. Per ciascuna
voce chiama il metodo `run()` dell'oggetto istruzione block, passando il `rt_prog_ctx_t`. Il
campo contatore di programma del contesto di runtime (`runtime_pc`) viene aggiornato dopo ogni
istruzione; le istruzioni di salto (`Goto`, `Gosub`, `Return`, le chiamate di procedura) lo
modificano direttamente per reindirizzare l'esecuzione.

---

### 9.3 Token e il Tokenizer

Prima che qualsiasi riga venga analizzata, `tokenizer_t` esegue l'analisi lessicale: esegue
la scansione della stringa sorgente grezza e produce una sequenza di oggetti `token_t`.

Ogni `token_t` contiene:

| Attributo | Membro | Descrizione |
|---|---|---|
| Testo originale | `_org_id` | Preserva la capitalizzazione originale |
| Testo normalizzato | `_identifier` | Versione in minuscolo, usata per il riconoscimento delle parole chiave |
| Classe del token | `_type` (enum `tkncl_t`) | Categoria del token |
| Posizione nel sorgente | `_position` | Offset in byte all'interno della riga sorgente |
| Puntatore all'espressione | `_expression_ptr` | Puntatore condiviso alla stringa dell'espressione circostante |

L'enumerazione `tkncl_t` definisce le categorie dei token:

```cpp
enum class tkncl_t {
    UNDEFINED,
    BLANK,
    NEWLINE,
    IDENTIFIER,    // keywords and variable names
    INTEGRAL,      // integer literals
    REAL,          // floating-point literals
    OPERATOR,      // +  -  *  /  ^  =  <>  <  <=  >  >=  \  &
    SUBEXP_BEGIN,  // (
    SUBEXP_END,    // )
    STRING_LITERAL,
    STRING_COMMENT,
    SUBSCR_BEGIN,  // [  (array subscript open)
    SUBSCR_END,    // ]
    LINE_COMMENT   // '  or  Rem
};
```

Il Parser di istruzioni usa `_type` per prendere decisioni di ramificazione rapida — vedere un
token `IDENTIFIER` il cui `_identifier` è `"print"` instrada immediatamente a `parse_print()`
— senza esaminare nuovamente la stringa originale.

Il design con enum piatto (anziché una gerarchia di classi per i tipi di token) fu una scelta
deliberata: mantiene tutti gli oggetti `token_t` omogenei e facilmente memorizzabili in un
contenitore standard.

---

### 9.4 Il Contenitore Lista di Token

La classe `token_list_t` avvolge una `std::deque<token_t>` e aggiunge operazioni di convenienza
che semplificano il parser:

- Rimozione efficiente da entrambe le estremità (estrazione dalla testa man mano che i token
  vengono consumati).
- Inserimento di marcatori: i token `SUBEXP_BEGIN`/`SUBEXP_END` vengono inseriti dal parser di
  espressioni per delimitare le sotto-espressioni dopo il riordinamento per precedenza degli
  operatori.
- Funzioni ausiliarie per esaminare il token in testa, verificare se la lista è vuota e saltare
  gli spazi bianchi.

Incapsulando la deque, `token_list_t` permette al codice del parser di essere letto come una
sequenza di operazioni `consuma-e-verifica` su uno stream di token, piuttosto che come
aritmetica di indici su un array grezzo.

---

### 9.5 Analisi del Codice

Tre parser distinti gestiscono diversi livelli della grammatica.

**Parser di espressioni (`expr_parser_t`)** — analizza una singola espressione come
`2 + Sin(PI() / 2)` e restituisce un `expr_any_t::handle_t`: un puntatore condiviso alla
radice di un albero sintattico valutabile. I suoi punti di ingresso sono:

```cpp
expr_any_t::handle_t compile(expr_tknzr_t& tknzr);
expr_any_t::handle_t compile(token_list_t tl, size_t expr_pos);
```

**Parser di istruzioni (`stmt_parser_t`)** — analizza una riga sorgente completa e restituisce
un `stmt_t::handle_t`. I suoi principali punti di ingresso sono:

```cpp
stmt_t::handle_t compile_line(prog_ctx_t& ctx, const std::string& source_line);
stmt_t::handle_t parse_block(prog_ctx_t& ctx, token_list_t& tl, ...);
stmt_t::handle_t parse_stmt (prog_ctx_t& ctx, token_list_t& tl);
```

`parse_block()` itera finché la lista di token non è vuota, chiamando `parse_stmt()` a ogni
iterazione. `parse_stmt()` esamina il primo token e smista verso un metodo dedicato
`parse_xxx()` per ciascuna parola chiave di istruzione: `parse_print()`,
`parse_for_to_step()`, `parse_if()`, `parse_sub()`, e così via.

Ogni volta che un metodo `parse_xxx()` incontra un'espressione nello stream di token, chiama
il Parser di espressioni per compilare quell'espressione in un nodo `expr_any_t`. Il nodo
risultante viene incorporato nell'oggetto istruzione; il suo metodo `eval()` verrà chiamato a
runtime.

**Parser CLI (dentro `interpreter_t`)** — gestisce i comandi specifici dell'interprete (`List`,
`Run`, `Load`, `Save`, `New`, `Help`, …). Questi comandi non sono istruzioni del linguaggio
BASIC; vengono riconosciuti e smistati da `interpreter_t::exec_command()`.

---

### 9.6 Analisi delle Espressioni in Dettaglio

L'analisi delle espressioni è dove avviene la maggior parte del lavoro algoritmico
interessante. La sfida centrale è la precedenza degli operatori: l'espressione `2 + 4 * 17`
deve valutare a `2 + (4 * 17) = 70`, non a `(2 + 4) * 17 = 102`.

Il Parser di espressioni risolve questo in tre passi.

**Passo 1 — Tokenizzazione.** La stringa dell'espressione viene passata a un `tokenizer_t`
(oppure viene riutilizzata la `token_list_t` già costruita) per produrre una sequenza piatta
di token.

**Passo 2 — Riordinamento per precedenza.** La lista di token viene ristrutturata per
codificare l'ordine di precedenza degli operatori. I token marcatori `SUBEXP_BEGIN`/`SUBEXP_END`
vengono inseriti per delimitare le sotto-espressioni. Funzioni ausiliarie statiche in
`expr_parser_t` — `reduce_brackets()`, `fix_real_numbers()`,
`fix_minus_prefixed_expressions()` — eseguono passate di normalizzazione. L'ordine di
precedenza applicato è:

1. Identità e negazione unaria (`+`, `-`)
2. Esponenziazione (`^`)
3. Moltiplicazione e divisione in virgola mobile (`*`, `/`)
4. Divisione intera (`\`, `Div`)
5. Modulo (`Mod`)
6. Addizione e sottrazione (`+`, `-`)
7. Confronto (`=`, `<>`, `<`, `<=`, `>`, `>=`)
8. Logici e bit a bit (`And`, `Or`, `Xor`, e le varianti bit a bit)

**Passo 3 — Costruzione dell'albero sintattico.** `expr_parser_t::parse()` costruisce
ricorsivamente un albero di nodi `expr_any_t`. Per `2 + 4 * 17` il risultato è:

```
        binary_expression(+, sum)
              /          \
        literal          binary_expression(*, multiplication)
        (integer, 2)           /        \
                          literal      literal
                          (int, 4)     (int, 17)
```

La gerarchia `expr_any_t` ha un tipo di nodo per ciascun elemento di espressione:

- **Espressione binaria** — un operatore, due nodi figli `expr_any_t` (sinistro e destro).
- **Espressione unaria** — un operatore, un nodo figlio.
- **Chiamata di funzione** — funzione integrata o definita dall'utente, con una lista di
  argomenti di nodi `expr_any_t`.
- **Variabile** — un nome cercato nel contesto di runtime al momento di `eval()`.
- **Costante letterale** — un valore intero, in virgola mobile o stringa incorporato al momento
  della compilazione.
- **Espressione vuota** — un sentinel senza operazione.

L'interfaccia della classe base è minimale:

```cpp
// include/nu_expr_any.h
class expr_any_t {
public:
    using handle_t    = std::shared_ptr<expr_any_t>;
    using func_args_t = std::vector<expr_any_t::handle_t>;

    virtual variant_t eval(rt_prog_ctx_t& ctx) const = 0;
    virtual bool      empty()  const noexcept = 0;
    virtual std::string name() const noexcept = 0;
    virtual ~expr_any_t() {}
};
```

Ogni nodo concreto sovrascrive `eval()` per eseguire il suo calcolo specifico e restituire un
`variant_t`. Il contesto di runtime `rt_prog_ctx_t& ctx` fornisce accesso ai valori delle
variabili, allo stack di chiamata e agli handle I/O.

---

### 9.7 Il Tipo Variant

`variant_t` (`include/nu_variant.h`) è il tipo valore universale del runtime di nuBASIC.
Ogni espressione valuta a un `variant_t`; ogni variabile ne memorizza uno.

I tipi supportati sono identificati dall'enumerazione `variant_t::type_t`:

| Enumeratore | Significato |
|---|---|
| `UNDEFINED` | Non inizializzato / non impostato |
| `INTEGER` | Intero con segno a 32 bit |
| `DOUBLE` | Virgola mobile a doppia precisione |
| `STRING` | Stringa di testo |
| `BYTEVECTOR` | Array di byte grezzo |
| `BOOLEAN` | Valore booleano |
| `STRUCT` | Struttura definita dall'utente |
| `OBJECT` | Handle di oggetto (per oggetti GUI/esterni) |
| `ANY` | Jolly (usato nelle firme delle procedure) |

Internamente, interi, double, booleani e stringhe scalari usano storage inline, quindi le
operazioni aritmetiche comuni, i confronti e le stringhe brevi non allocano piu' un vettore
di appoggio con un solo elemento. Le variabili array continuano a usare il payload vettoriale,
perche' richiedono storage indicizzato. I metadati di strutture e oggetti sono contenuti in un
payload separato, mantenendo piu' leggeri i valori non-struct. Le copie dei payload struct
usano copy-on-write: passare o assegnare una struct e' economico finche' una copia non viene
modificata. L'ordine dei campi struct segue l'ordine di dichiarazione, cosa importante per
l'interoperabilita' nativa e per un'introspezione runtime prevedibile.

Usare `variant_t` anziché una `union` in stile C o una gerarchia di classi per ciascun tipo
semplifica enormemente il valutatore: ogni operazione aritmetica, confronto, funzione su
stringhe e assegnamento lavora con lo stesso tipo, e le coercioni di tipo implicite (da intero
a double, da intero a stringa, ecc.) sono gestite in un unico posto.

---

### 9.8 Traccia dell'Esecuzione di un Programma Semplice

Di seguito si traccia il percorso completo dall'input da tastiera all'output stampato per il
programma a riga singola:

```basic
10 Print 2 + 4 * 17
```

**1. Punto di ingresso.**
Dopo che il programma è stato inserito nel REPL, l'utente digita `RUN`. Il ciclo di lettura
della console ottiene la stringa `"RUN"` dallo standard input e chiama
`interpreter_t::exec_command("RUN")`.

**2. Fase di build.**
`exec_command()` riconosce `"RUN"` e chiama `interpreter_t::rebuild()`.
`rebuild()` azzera `_prog_line` e il contesto statico (`prog_ctx_t`), poi itera su
ogni voce in `_source_line`. Per la riga 10 chiama
`stmt_parser_t::compile_line(ctx, "10 PRINT 2+4*17")`.

`compile_line()` crea un `tokenizer_t` dalla stringa sorgente e chiama `parse_block()`.
Il tokenizer produce:

```
{ ("print", IDENTIFIER), (" ", BLANK),
  ("2", INTEGRAL), ("+", OPERATOR),
  ("4", INTEGRAL), ("*", OPERATOR), ("17", INTEGRAL) }
```

`parse_block()` chiama `parse_stmt()`. Il primo token significativo è `"print"` → smista
verso `parse_print()`.

**3. Costruzione dell'istruzione Print.**
`parse_print()` chiama il template ausiliario `parse_arg_list()`. Questo ausiliario guida il
Parser di espressioni sui token rimanenti `2 + 4 * 17`. Il Parser di espressioni riordina la
lista di token per codificare la precedenza e poi costruisce l'albero:

```
binary_expression(+)
 ├── literal(integer, 2)
 └── binary_expression(*)
      ├── literal(integer, 4)
      └── literal(integer, 17)
```

Il `expr_any_t::handle_t` radice viene avvolto in una lista di argomenti e memorizzato dentro
un nuovo oggetto `stmt_print_t`. `parse_print()` restituisce un `stmt_t::handle_t` che punta
a questo oggetto.

`parse_block()` lo avvolge in un handle di istruzione block e restituisce. `compile_line()`
memorizza il risultato in `_prog_line[10]`.

**4. Fase di esecuzione.**
`rebuild()` termina e `run()` crea un oggetto `program_t` da `_prog_line` e `rt_prog_ctx_t`.
Il programma itera sulla sua mappa di righe; per la riga 10 chiama `stmt_print_t::run(ctx)`.

**5. Valutazione.**
`stmt_print_t::run()` itera sulla sua lista di argomenti. Il singolo argomento è il nodo
`expr_any_t` radice. Chiama `node->eval(ctx)`.

`eval()` sul `binary_expression(+)` chiama `eval()` sui suoi due figli:

- Il figlio sinistro `literal(2)` restituisce `variant_t(INTEGER, 2)`.
- Il figlio destro `binary_expression(*)` valuta a `variant_t(INTEGER, 68)` (4 × 17).

Il nodo `+` li somma: `variant_t(INTEGER, 70)`.

**6. Output.**
`stmt_print_t::run()` riceve `variant_t(INTEGER, 70)`, lo converte nella stringa `"70"` e lo
scrive nello stream di output. La console visualizza:

```
70
```

---

### 9.9 Estendere l'Insieme delle Funzioni Integrate

Aggiungere una nuova funzione integrata richiede di modificare esattamente quattro file
sorgente. Le modifiche sono autonome: non è necessario modificare regole del parser né è
coinvolta alcuna grammatica. L'interprete scopre le nuove funzioni interamente attraverso le
strutture dati popolate in quei quattro file.

L'esempio in questa sezione è una funzione di convoluzione 1-D `Conv`. Dati due vettori
*u* = (1, 0, 1) e *v* = (2, 7) — che rappresentano rispettivamente i coefficienti polinomiali
di *x*² + 1 e 2*x* + 7 — `Conv(u, v)` dovrebbe restituire il vettore (2, 7, 2, 7), cioè i
coefficienti di 2*x*³ + 7*x*² + 2*x* + 7. Un programma nuBASIC che usa la nuova funzione si
presenta così:

```basic
Dim u(3) As Double
Dim v(2) As Double
u(0)=1 : u(1)=0 : u(2)=1
v(0)=2 : v(1)=7
Dim w(4) As Double
w = Conv(u, v)
For i = 0 To 3
    Print w(i);" ";
Next i
' output: 2 7 2 7
```

`Conv` sarà definita con due overload:

```
Conv( v1(), v2() )                    ' use full array sizes
Conv( v1(), v2(), count1, count2 )    ' use only the first count1/count2 elements
```

#### I quattro file da modificare

| File | Cosa aggiungere |
|---|---|
| `lib/nu_global_function_tbl.cc` | functor C++ + `fmap["conv"] = conv_functor;` |
| `lib/nu_builtin_help.cc` | Una voce in `_help_content[]` |
| `lib/nu_reserved_keywords.cc` | `"conv"` nell'insieme `list()` |

> **Nota:** `Conv` è già presente nella lista delle parole chiave distribuita e nella tabella
> della guida — è stata usata come implementazione di riferimento quando questi punti di
> estensione furono progettati per la prima volta. La procedura guidata che segue ricostruisce
> il codice esatto che risiede in quei file.

---

#### Passo 1 — Implementare l'algoritmo C++

La convoluzione C++ pura è indipendente dall'interprete e facile da testare in isolamento:

```cpp
template <typename T>
std::vector<T> conv(const std::vector<T>& v1, const std::vector<T>& v2)
{
    const int n = int(v1.size());
    const int m = int(v2.size());
    const int k = n + m - 1;
    std::vector<T> w(k, T());

    for (int i = 0; i < k; ++i) {
        const int jmn = (i >= m - 1) ? i - (m - 1) : 0;
        const int jmx = (i < n - 1)  ? i            : n - 1;
        for (int j = jmn; j <= jmx; ++j)
            w[i] += v1[j] * v2[i - j];
    }
    return w;
}
```

Inserire questo template vicino alla parte superiore di `lib/nu_global_function_tbl.cc`, prima
della funzione `get_instance()`.

---

#### Passo 2 — Scrivere il functor

Ogni funzione integrata è registrata come un callable C++ con la firma:

```cpp
variant_t functor_name(
    rt_prog_ctx_t&        ctx,   // runtime execution context
    const std::string&    name,  // function name (one functor can serve several names)
    const nu::func_args_t& args  // vector of unevaluated expression nodes
);
```

`func_args_t` è `std::vector<expr_any_t::handle_t>`. Valutare `args[i]->eval(ctx)` discende
nel valutatore dell'albero di espressioni descritto nel §9.6 e restituisce un `variant_t`.

L'implementazione completa di `conv_functor`:

```cpp
static variant_t conv_functor(
    rt_prog_ctx_t&         ctx,
    const std::string&     name,
    const nu::func_args_t& args)
{
    // 1. Get and validate argument count (2 or 4)
    const auto args_num = args.size();
    rt_error_code_t::get_instance().throw_if(
        args_num != 2 && args_num != 4,
        0, rt_error_code_t::value_t::E_INVALID_ARGS, "");

    // 2. Evaluate the first two arguments (the input vectors)
    //    Left-to-right evaluation order is mandatory — each eval() call may
    //    have side effects on the shared runtime context.
    auto variant_v1 = args[0]->eval(ctx);
    auto variant_v2 = args[1]->eval(ctx);

    const auto actual_v1_size = variant_v1.vector_size();
    const auto actual_v2_size = variant_v2.vector_size();

    // 3. Resolve logical sizes (from optional 3rd/4th arguments, or full array)
    const size_t size_v1 =
        args_num == 4 ? size_t(args[2]->eval(ctx).to_long64()) : actual_v1_size;
    const size_t size_v2 =
        args_num == 4 ? size_t(args[3]->eval(ctx).to_long64()) : actual_v2_size;

    // 4. Validate: logical sizes must be within actual array bounds and >= 1
    rt_error_code_t::get_instance().throw_if(
        size_v1 > actual_v1_size || size_v1 < 1,
        0, rt_error_code_t::value_t::E_INV_VECT_SIZE, args[0]->name());
    rt_error_code_t::get_instance().throw_if(
        size_v2 > actual_v2_size || size_v2 < 1,
        0, rt_error_code_t::value_t::E_INV_VECT_SIZE, args[1]->name());

    // 5. Copy variant contents into plain C++ vectors of double
    std::vector<double> v1(size_v1);
    std::vector<double> v2(size_v2);

    bool ok = variant_v1.copy_vector_content(v1);
    rt_error_code_t::get_instance().throw_if(
        !ok, 0, rt_error_code_t::value_t::E_INV_VECT_SIZE, args[0]->name());

    ok = variant_v2.copy_vector_content(v2);
    rt_error_code_t::get_instance().throw_if(
        !ok, 0, rt_error_code_t::value_t::E_INV_VECT_SIZE, args[1]->name());

    // 6. Compute and return
    auto vr = conv(v1, v2);
    const nu::variant_t result(std::move(vr));
    return result;
}
```

Punti chiave:

- `rt_error_code_t::get_instance().throw_if(condition, line, code, token)` lancia
  un'eccezione di runtime strutturata quando `condition` è vera. L'interprete la intercetta,
  formatta il messaggio di errore da `lib/nu_error_codes.cc` e lo visualizza all'utente.
  `E_INVALID_ARGS` segnala un numero o tipo di argomenti errato; `E_INV_VECT_SIZE` segnala
  che un argomento vettore ha un'incoerenza di dimensione.
- Gli argomenti vengono valutati **in ordine** (`args[0]`, poi `args[1]`, …). Ogni chiamata
  `eval()` può aggiornare variabili o far avanzare lo stato I/O, quindi l'ordine è
  semanticamente significativo.
- `variant_t::copy_vector_content(std::vector<double>&)` riempie il vettore di destinazione
  con la rappresentazione in doppia precisione di ciascun elemento e restituisce `false` se il
  variant non è un vettore o la destinazione è troppo piccola.
- Il costruttore `variant_t(std::move(vr))` (dichiarato in `include/nu_variant.h`) sposta il
  `std::vector<double>` risultante direttamente nel variant senza copiarlo.

---

#### Passo 3 — Registrare il functor

Dentro `global_function_tbl_t::get_instance()` in `lib/nu_global_function_tbl.cc`, aggiungere
una riga al blocco `fmap`:

```cpp
fmap["conv"] = conv_functor;
```

`fmap` è il riferimento `global_function_tbl_t&` restituito da `get_instance()`. La chiave è
un `icstring_t` (stringa senza distinzione tra maiuscole e minuscole), quindi gli utenti di
nuBASIC possono scrivere `Conv`, `conv` o `CONV` indifferentemente.

---

#### Passo 4 — Aggiungere la guida inline

In `lib/nu_builtin_help.cc`, aggiungere una voce all'array statico `_help_content[]`:

```cpp
{ lang_item_t::FUNCTION, "conv",
    "Returns a vector of Double as result of convolution of 2 given vectors of numbers",
    "Conv( v1, v2 [, count1, count2 ] )" },
```

I quattro campi di `help_content_t` sono:

| Campo | Tipo | Scopo |
|---|---|---|
| `lang_item` | enum `lang_item_t` | `FUNCTION`, `INSTRUCTION`, `COMMAND`, o `OPERATOR` |
| `lang_id` | `const char*` | Nome come appare nell'output di `Help` (confrontato senza distinzione tra maiuscole e minuscole) |
| `description` | `const char*` | Descrizione in forma libera; `\n` produce una nuova riga |
| `signature` | `const char*` | Sintassi di chiamata mostrata sotto la descrizione |

Dopo questa modifica, il REPL risponde a:

```
Help Conv
Apropos convolution
```

---

#### Passo 5 — Aggiungere alla lista delle parole chiave riservate

In `lib/nu_reserved_keywords.cc`, aggiungere `"conv"` (in minuscolo) all'insieme restituito
da `reserved_keywords_t::list()`:

```cpp
std::set<std::string>& reserved_keywords_t::list()
{
    static std::set<std::string> list = {
        // ...existing keywords...
        "conv",
        // ...
    };
    return list;
}
```

Registrare un nome qui indica al tokenizer di classificarlo come identificatore parola chiave
(`tkncl_t::IDENTIFIER`) e lo rende disponibile al motore di autocompletamento nell'IDE. I
nomi di funzioni integrate che *non* si trovano in questa lista funzionano comunque a runtime
ma non vengono evidenziati nell'editor e non compaiono nella lista di autocompletamento F12.

---

#### Riepilogo: lista di controllo dei quattro file

| File | Modifica |
|---|---|
| `lib/nu_global_function_tbl.cc` | 1. Aggiungere il template dell'algoritmo C++. 2. Aggiungere il functor. 3. Aggiungere `fmap["conv"] = conv_functor;` dentro `get_instance()`. |
| `lib/nu_builtin_help.cc` | Aggiungere una voce `help_content_t` a `_help_content[]`. |
| `lib/nu_reserved_keywords.cc` | Aggiungere `"conv"` all'insieme `list()`. |

Ricompilare il progetto. La nuova funzione è immediatamente disponibile per qualsiasi programma
nuBASIC, restituisce un vettore `variant_t` correttamente tipizzato, solleva errori strutturati
su input non validi ed è completamente documentata nel sistema di guida inline.

---

## 10. Compilare nuBASIC dai Sorgenti

nuBASIC è scritto in C++20 e si compila con CMake 3.14 o successivo. Lo stesso albero dei
sorgenti produce tre artefatti distinti a seconda della piattaforma di destinazione:

| Artefatto | Piattaforma | Descrizione |
|---|---|---|
| `nubasic.exe` | Windows | CLI **console-subsystem** — eredita stdio, rispetta pipe e redirezioni. E' il binario di default per uso batch, scripting, CI e VS Code |
| `nubasicgdi.exe` | Windows | CLI **GDI-subsystem** — apre una finestra GDI dedicata per grafica, audio e mouse. E' il binario storico, utile per programmi grafici lanciati da una scorciatoia |
| `nubasicdebug.exe` | Windows | Backend di debug console-subsystem usato da VS Code (sia in modalita' testo che ibrida grafica). Espone il protocollo machine-interface su stdout |
| `NuBasicIDE.exe` + `SciLexer.dll` | Windows | IDE grafico con debugger integrato |
| `nubasic` | Linux/macOS | Interprete da riga di comando (CLI) |
| `nubasicdebug` | Linux/macOS | Backend di debug usato da VS Code |
| `nubasicide` | Linux | IDE grafico GTK+2 |

Su Windows, lo split tra `nubasic.exe` (console) e `nubasicgdi.exe` (GDI)
e' stato introdotto perche' il **binario console e' il default**: si
integra correttamente con `cmd.exe`, PowerShell, shell MSYS, redirezioni
e tooling che assume stdio standard. I programmi che vogliono ancora la
finestra GDI tradizionale (o le istruzioni grafiche senza la modalita'
ibrida `--graphics-window`) lanciano `nubasicgdi.exe`.

Un build minimale — solo interprete, senza grafica né IDE — è disponibile anche per ambienti
embedded e minimali su Linux/macOS.

---

### 10.1 Ottenere il Codice Sorgente

Clona il repository Git:

```
git clone https://github.com/eantcal/nubasic.git
cd nubasic
```

In alternativa, scarica un archivio di rilascio da
[github.com/eantcal/nubasic/releases](https://github.com/eantcal/nubasic/releases) e
decomprimi il contenuto.

---

### 10.2 Panoramica dei Target di Build

Il grafo di build CMake include i seguenti target:

| Target | Tipo | Piattaforme | Dipende da |
|---|---|---|---|
| `nuBasicInterpreter` | libreria statica | tutte | *(libreria radice)* |
| `nuWinConsole` | libreria statica | Windows | GDI (Windows SDK) |
| `nuBasicCLI` (`nubasic.exe`) | eseguibile | Windows | `nuBasicInterpreter` — console subsystem |
| `nuBasicGDI` (`nubasicgdi.exe`) | eseguibile | Windows | `nuBasicInterpreter`, `nuWinConsole` — GDI subsystem |
| `nuBasicDebugCLI` (`nubasicdebug.exe`) | eseguibile | Windows | `nuBasicInterpreter` — console subsystem, backend del debugger per VS Code |
| `nuBasicIDE` (`NuBasicIDE.exe`) | eseguibile | Windows | `nuBasicInterpreter`, `nuWinConsole`, `SciLexer` |
| `SciLexer` | libreria condivisa (`.dll`) | Windows | Scintilla 5.5.3 + Lexilla 5.4.3 |
| `nubasic` | eseguibile | Linux/macOS | `nuBasicInterpreter`, X11 |
| `nubasicdebug` | eseguibile | Linux/macOS | `nuBasicInterpreter` — backend del debugger per VS Code |
| `nubasicide` | eseguibile | Linux | `nuBasicInterpreter`, GTK+2, Scintilla (statica) |
| `scintilla` | libreria statica | Linux | GTK+2 |
| `mipjson` / `miptknzr` | librerie statiche | Linux | *(configurazione JSON per l'IDE GTK)* |

Scintilla 5.5.3 e Lexilla 5.4.3 vengono scaricate automaticamente tramite `FetchContent` di
CMake alla prima configurazione della build (richiede accesso a Internet). Per usare la
versione bundled di Scintilla 4, passa `-DSCINTILLA_LOCAL=ON`.

---

### 10.3 Build su Windows

#### Prerequisiti

- **Visual Studio 2022** (Community o superiore) con il carico di lavoro *Sviluppo di applicazioni desktop con C++*,
  oppure una toolchain MSVC equivalente.
- **CMake 3.14+** — incluso in Visual Studio 2022, oppure installabile separatamente.
- **Git** — per clonare il repository e per consentire a `FetchContent` di CMake di scaricare Scintilla.
- **WiX Toolset 3.x** *(opzionale)* — richiesto solo per creare il MSI installer.

#### Configurazione e build con Visual Studio

Apri il *Prompt dei comandi per gli sviluppatori di VS 2022*, poi esegui:

```bat
cd nubasic
mkdir build && cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

I binari Release vengono collocati in `build\Release\`:

```
build\Release\
├── NuBasicIDE.exe
├── nubasic.exe
└── SciLexer.dll
```

Per aprire la soluzione nell'IDE di Visual Studio:

```bat
cmake -G "Visual Studio 17 2022" ..
start nuBASIC.sln
```

Seleziona il progetto di avvio `nuBasicIDE` o `nuBasicCLI` e premi **F5**.

#### Configurazione e build con Ninja (MSVC)

Se preferisci un build più rapido con Ninja da un *Prompt dei comandi per gli sviluppatori*:

```bat
cd nubasic
mkdir build && cd build
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

> **Nota:** Il file `CMakeLists.txt` radice di CMake include il rilevamento automatico dei
> percorsi degli header MSVC e del Windows SDK quando si usa Ninja senza `vcvarsall.bat`. Se
> compaiono errori del compilatore relativi a header mancanti, apri un *Prompt dei comandi per
> gli sviluppatori* (che esegue `vcvarsall.bat` automaticamente) prima di invocare CMake.

#### Build minimale (build minimale) su Windows

Per compilare solo `nubasic.exe` senza l'IDE:

```bat
cmake -G "Visual Studio 17 2022" -DWITH_WIN_IDE=OFF ..
cmake --build . --config Release
```

---

### 10.4 Build su Linux

#### Prerequisiti — build completa (interprete + IDE GTK)

Installa il compilatore e le librerie necessarie. Su Debian/Ubuntu:

```sh
sudo apt-get update
sudo apt-get install build-essential cmake git \
    libx11-dev \
    libgtk2.0-dev libglib2.0-dev libpango1.0-dev \
    libatk1.0-dev libcairo2-dev \
    xterm xmessage
```

> `xterm` e `xmessage` vengono utilizzati a runtime da alcune funzioni built-in di nuBASIC
> (`Shell`, `MsgBox`). `aplay` è usato per `PlaySound`; installalo tramite `alsa-utils` se
> necessario.

Su Fedora/RHEL/CentOS:

```sh
sudo dnf install gcc-c++ cmake git \
    libX11-devel \
    gtk2-devel glib2-devel pango-devel \
    atk-devel cairo-devel \
    xterm xmessage
```

#### Configurazione e build

```sh
cd nubasic
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

Questo produce:

```
/usr/local/bin/nubasic      # interprete da console
/usr/local/bin/nubasicide   # IDE GTK
```

#### Build minimale su Linux

Per compilare solo l'interprete da riga di comando senza X11 o GTK:

```sh
cmake .. -DWITH_X11=OFF -DWITH_IDE=OFF
make -j$(nproc)
```

Questo attiva il flag del preprocessore `TINY_NUBASIC_VER` (modalità build console), che rimuove
tutti i componenti grafici, audio e IDE. Il binario `nubasic` risultante non ha dipendenze esterne
oltre al runtime C++.

#### Solo IDE, senza build da console

Se vuoi l'IDE GTK ma non l'interprete da console:

```sh
cmake .. -DWITH_X11=ON -DWITH_IDE=ON
```

Entrambi i flag sono `ON` per impostazione predefinita su Linux, quindi un semplice `cmake ..`
produce già entrambi i binari. Usa `-DWITH_IDE=OFF` per escludere `nubasicide` se GTK+2 non
è disponibile.

---

### 10.5 Build su macOS

Il supporto a macOS è allo stesso livello del build da console Linux. GTK+2 e le funzionalità
dipendenti da X11 non sono disponibili; `WITH_X11` e `WITH_IDE` hanno entrambi come valore
predefinito `OFF` su macOS.

#### Prerequisiti

Installa Xcode Command Line Tools, CMake e libffi:

```sh
xcode-select --install
brew install cmake git libffi
```

> `libffi` è necessario per le chiamate a librerie native (`Declare Function … Lib "…"`).

#### Configura e compila

```sh
cd nubasic
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(sysctl -n hw.logicalcpu)
sudo make install   # installa nubasic e nubasicdebug in /usr/local/bin
```

#### Disinstallazione su macOS

```sh
sudo xargs rm -f < build/install_manifest.txt
```

---

### 10.6 Build su iOS con iSH

[iSH](https://ish.app) è un ambiente shell Alpine Linux per iOS/iPadOS. Supporta il build
minimale da console di nuBASIC.

Installa iSH dall'App Store, poi all'interno della sua shell:

```sh
apk add g++ make cmake git

git clone https://github.com/eantcal/nubasic.git
cd nubasic
mkdir build && cd build
cmake .. -DWITH_X11=OFF -DWITH_IDE=OFF
make -j2
./nubasic
```

I flag `-DWITH_X11=OFF -DWITH_IDE=OFF` sono obbligatori perché iSH non dispone di un server
grafico.

---

### 10.7 Riferimento alle Opzioni CMake

Tutte le opzioni possono essere passate dalla riga di comando di `cmake` come `-D<OPTION>=<VALUE>`.

| Opzione | Valore predefinito | Piattaforma | Descrizione |
|---|---|---|---|
| `WITH_X11` | `ON` | Linux | Abilita la grafica X11, l'audio e l'integrazione con xterm/xmessage. Imposta `OFF` per un build headless o embedded. |
| `WITH_IDE` | `ON` | Linux | Compila l'IDE GTK+2 (`nubasicide`). Richiede `WITH_X11=ON` e le librerie di sviluppo GTK+2. |
| `WITH_WIN_IDE` | `ON` | Windows | Compila l'IDE Windows (`NuBasicIDE.exe`). Imposta `OFF` per compilare solo `nubasic.exe`. |
| `SCINTILLA_LOCAL` | `OFF` | IDE Windows/Linux | Usa il sorgente Scintilla 4.x incluso nel repository sotto `ide/scintilla/` invece di scaricare Scintilla 5.5.3. |
| `SCINTILLA_VERSION` | `"5.5.3"` | IDE Windows/Linux | Versione di Scintilla da scaricare quando `SCINTILLA_LOCAL=OFF`. |
| `LEXILLA_VERSION` | `"5.4.3"` | IDE Windows/Linux | Versione di Lexilla da scaricare quando `SCINTILLA_LOCAL=OFF`. |
| `NUBASIC_INSTALLER` | `"WIX"` | Windows | Generatore di installer per CPack: `"WIX"` (MSI, consigliato) oppure `"NSIS"` (setup.exe legacy). |
| `NUBASIC_WITH_RAYCAST` | `ON` (Windows), `OFF` (altre) | tutte | Compila il motore pseudo-3D WinRayCast integrato e l'API `Ray…` (vedi [sezione 5.5](#55-pseudo-3d-con-winraycast)). |
| `NUBASIC_WITH_LIBFFI` | `ON` | tutte | Abilita le chiamate native a DLL/oggetti condivisi tramite libffi (`Declare Function … Lib`). Senza libffi le dichiarazioni vengono analizzate ma l'invocazione è disabilitata. |
| `CMAKE_BUILD_TYPE` | `Release` | tutte | `Release` (ottimizzato, `-O3`) oppure `Debug` (con simboli, `-g`). |

I preset di `CMakePresets.json` `release` / `debug` compilano la configurazione standard;
`raycast-release` / `raycast-debug` abilitano WinRayCast esplicitamente. Configura con
`cmake --preset <name>` e compila con `cmake --build --preset <name>`.

#### Esempio: build di debug dell'IDE Windows senza scaricare Scintilla

```bat
cmake -G "Visual Studio 17 2022" ^
      -DSCINTILLA_LOCAL=ON ^
      ..
cmake --build . --config Debug
```

#### Esempio: build Linux minimale senza dipendenze esterne

```sh
cmake .. -DWITH_X11=OFF -DWITH_IDE=OFF -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

---

### 10.8 Creazione di Installer e Pacchetti

#### MSI installer per Windows

##### Prerequisiti per l'installer Windows

| Strumento | Dove scaricarlo |
| --------- | --------------- |
| Visual Studio 2022 (workload Desktop C++) | [visualstudio.microsoft.com](https://visualstudio.microsoft.com) |
| WiX Toolset v3.x (`candle.exe` + `light.exe` nel PATH) | [wixtoolset.org/releases](https://wixtoolset.org/releases/) |
| Node.js 18 LTS+ *(opzionale — solo per il componente VS Code extension)* | [nodejs.org](https://nodejs.org) |

> **Alternativa NSIS** — passa `-DNUBASIC_INSTALLER=NSIS` a cmake e installa [NSIS](https://nsis.sourceforge.io/) al posto di WiX.

##### Generare la soluzione Visual Studio

```bat
cd nubasic
mkdir build-win && cd build-win
cmake -G "Visual Studio 17 2022" -A x64 ..
```

##### Tramite Visual Studio (consigliato)

Apri `nuBASIC.sln`, imposta la configurazione su **Release**, poi in Solution Explorer:

1. Espandi la cartella **Installer**.
2. Clic destro su **`CreateInstaller`** → **Build**.

Il file MSI viene scritto nella directory di build come `nuBASIC_<versione>.msi`.
Il target `PACKAGE` (anch'esso nella cartella Installer) esegue il pipeline CPack standard senza il passo di patch dei shortcut WiX.

`CreateInstaller` chiama internamente `cmake/RunCpackInstaller.ps1`, che esegue CPack e poi
`cmake/PatchWixMSI.ps1` per iniettare i collegamenti nel menu Start e sul Desktop nell'MSI.

Il programma di installazione espone quattro componenti opzionali tramite il dialog **WixUI_FeatureTree**:

| Componente | Obbligatorio | Descrizione |
| ---------- | ------------ | ----------- |
| **Core** | sì | `nubasic.exe` (console), `nubasicgdi.exe` (GDI), `nubasicdebug.exe` (debug backend) |
| **IDE** | opzionale | `NuBasicIDE.exe` + DLL Scintilla/Lexilla |
| **VS Code Extension** | opzionale | `nubasic-latest.vsix`; installata automaticamente in VS Code tramite `install-vscode-ext.ps1` |
| **Examples** | opzionale | File sorgente `.bas` di esempio |

##### Tramite riga di comando

```bat
cmake --build . --config Release
cmake --build . --target CreateInstaller --config Release
```

**Variante NSIS (setup.exe invece di MSI):**

```bat
cmake -G "Visual Studio 17 2022" -A x64 -DNUBASIC_INSTALLER=NSIS ..
cmake --build . --config Release
cmake --build . --target CreateInstaller --config Release
```

##### Installazione e disinstallazione su Windows

```bat
rem Installazione silenziosa
msiexec /i nuBASIC_2.0.0.msi /quiet /norestart

rem Disinstallazione silenziosa
msiexec /x nuBASIC_2.0.0.msi /quiet
```

---

#### Pacchetto DEB per Ubuntu / Debian

##### Tramite lo script (consigliato)

Lo script `create_ubuntu_package.sh` (nella radice del repository) installa le
dipendenze di build, configura, compila e impacchetta nuBASIC in un unico passo:

```sh
# Build completo (interprete console + IDE GTK2):
./create_ubuntu_package.sh

# Solo console (senza IDE GTK2):
./create_ubuntu_package.sh --console-only

# Directory di build personalizzata, 8 job paralleli:
./create_ubuntu_package.sh --build-dir /tmp/nubasic-deb --jobs 8

# Salta apt-get (dipendenze già installate):
./create_ubuntu_package.sh --no-install-deps
```

Se i pacchetti GTK2 non sono disponibili, lo script torna automaticamente a un build console-only.

##### Build manuale

```sh
cd nubasic
mkdir build-deb && cd build-deb
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
make -j$(nproc)
cpack -G DEB -D CPACK_PACKAGING_INSTALL_PREFIX=/usr
```

##### Installazione e disinstallazione su Ubuntu

```sh
# Ispezione del pacchetto:
dpkg-deb -I nuBASIC_2.0.0.deb

# Installazione:
sudo dpkg -i nuBASIC_2.0.0.deb
sudo apt-get install -f   # risolve eventuali dipendenze mancanti

# Disinstallazione:
sudo dpkg -r nubasic
```

Dopo l'installazione, `nubasic` e `nubasicdebug` si trovano in `/usr/bin`.
Nel build completo viene installato anche `nubasicide` (l'IDE GTK2).

---

#### macOS — installazione manuale

macOS non dispone di un programma di installazione pacchettizzato.
Compila e installa come descritto nella [sezione 10.5](#105-build-su-macos).

Per disinstallare:

```sh
sudo xargs rm -f < build/install_manifest.txt
```

---

*nuBASIC è open source con licenza MIT.*
*Segnalazioni di bug e richieste di funzionalità: antonino.calderone@gmail.com*

---
