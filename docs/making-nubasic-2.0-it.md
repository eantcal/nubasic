# Making nuBASIC 2.0: internals di runtime e debugger

Questa nota e' pensata per sviluppatori interessati a come nuBASIC 2.0 ha
migliorato il core dell'interprete, il modello di debug e la rappresentazione
dei valori a runtime. L'obiettivo e' spiegare perche' queste modifiche
migliorano qualita' del linguaggio, manutenibilita' e prestazioni.

## Obiettivi di design

nuBASIC 2.0 conserva lo spirito originale del progetto: un piccolo linguaggio
della famiglia BASIC capace di eseguire programmi classici con numeri di riga,
ma anche di supportare programmazione strutturata, classi, grafica, progetti,
chiamate native e workflow di debug moderni.

Il lavoro recente sugli internals aveva quattro obiettivi principali:

- Rendere esplicite e riusabili le semantiche di esecuzione del debugger.
- Separare l'output grafico dal protocollo del debugger e dall'I/O testuale.
- Ridurre allocazioni e copie nel tipo valore centrale `variant_t`.
- Rendere le feature del linguaggio piu' facili da documentare, testare e
  riusare da CLI, IDE, estensione VS Code e tooling futuro.

## Modello di esecuzione del debugger

Il debug storico di nuBASIC era basato soprattutto sul passo singolo a livello
di statement: esegui una istruzione BASIC, fermati, ripeti. E' utile, ma non
copre le aspettative di un debugger moderno.

Il core dell'interprete ora espone comandi di esecuzione con semantica propria:

- Continue
- Step Into
- Step Over
- Step Out
- Pause / Break
- Run to Cursor

La scelta importante e' che questi comandi sono concetti dell'interprete, non
della UI. L'IDE Scintilla, la debug CLI e l'adapter VS Code possono chiedere lo
stesso comando e ricevere eventi di stop coerenti.

Questo migliora la qualita' perche' il comportamento di stepping viene definito
una sola volta, vicino allo stato runtime da cui dipende: statement corrente,
nidificazione di procedure e funzioni, flusso GOSUB/RETURN, loop, profondita'
dello stack e breakpoint. I livelli UI non devono piu' indovinare come simulare
Step Over o Step Out.

## Debug di programmi grafici

I programmi grafici hanno bisogno di una superficie di disegno, ma i messaggi
del protocollo debugger e la diagnostica testuale non devono essere mescolati
con quella superficie. VS Code, in particolare, usa stdout per il protocollo del
debug adapter e per l'output di terminale.

Il backend di debug Windows supporta quindi una modalita' grafica ibrida:

- `nubasicdebug.exe` resta un processo console-subsystem.
- `--graphics-window` apre una finestra GDI separata per l'output grafico.
- Gli eventi machine-interface del debugger e la diagnostica testuale restano
  su stdout.
- Breakpoint, stepping, pausa e program-counter reporting restano nel normale
  flusso del debugger.

Questa separazione migliora sia l'esperienza utente sia l'affidabilita' del
tooling. Un gioco o una animazione possono essere debuggati senza occupare il
canale di comunicazione usato dall'IDE o dall'adapter VS Code.

## Ottimizzazione dei valori runtime

`variant_t` e' il tipo valore universale dell'interprete. Ogni espressione
valuta a un `variant_t`, e ogni variabile ne memorizza uno. Per questo e' un
percorso critico: aritmetica, confronti, stringhe, array, struct, oggetti,
chiamate a procedure e funzioni built-in passano tutti da questo tipo.

La rappresentazione precedente usava un payload vettoriale anche per i valori
scalari. Un semplice intero o double poteva pagare il costo di una allocazione
per un vettore con un solo elemento. Era una soluzione semplice, ma costosa nei
percorsi caldi come la valutazione di espressioni e gli assegnamenti composti.

La nuova rappresentazione conserva lo stesso comportamento pubblico ma migliora
il layout interno:

- Interi, double, booleani e stringhe scalari usano storage inline.
- Gli array continuano a usare il payload vettoriale, perche' richiedono
  storage indicizzato.
- I metadati di struct e oggetti sono contenuti in un payload separato, quindi
  i valori scalari ordinari non ne pagano il footprint.
- Le copie dei payload struct usano copy-on-write, quindi passaggio per valore
  e assegnamento sono economici finche' una copia non viene modificata.
- I campi delle struct conservano l'ordine di dichiarazione, cosa importante
  per interoperabilita' nativa e introspezione prevedibile.

Il beneficio per il linguaggio e' che semantiche valore piu' ricche diventano
meno costose. Classi, struct, distruttori, chiamate native e watch del debugger
possono usare lo stesso tipo valore senza far pagare al codice scalare il costo
dei casi piu' pesanti.

## Assegnamenti composti

Gli assegnamenti composti come `+=`, `-=`, `*=` e `/=` hanno ora percorsi di
mutazione diretta per i casi scalari comuni. Invece di calcolare sempre un
temporaneo e assegnarlo indietro, interi e double possono essere aggiornati
in-place quando non serve promozione di tipo.

Il fallback usa ancora gli operatori normali quando la promozione e' necessaria,
per esempio quando `Integer += Double` deve produrre un risultato double.
Questo mantiene prevedibile la semantica e rende piu' veloce il caso comune.

## Chiamate native e line continuation

Le chiamate a librerie native permettono al codice BASIC di raggiungere API di
piattaforma:

```basic
Declare Function GetCurrentProcessId Lib "kernel32.dll" () As DWORD
```

La feature usa libffi per l'invocazione e loader di piattaforma come
`LoadLibraryW` su Windows e `dlopen` su POSIX. E' abilitata per default in host
locali fidati e puo' essere disabilitata con `--disable-native-calls`.

La continuazione di riga con `_` e' stata aggiunta anche per rendere leggibili
dichiarazioni native e chiamate lunghe:

```basic
Declare Function MessageBoxA Lib "user32.dll" _
    Alias "MessageBoxA" _
    (hwnd As Pointer, text As String, caption As String, flags As DWORD) As Integer
```

Il parser unisce le righe fisiche continuate prima di analizzare lo statement
BASIC logico. I debugger si fermano sulla prima riga fisica dello statement
unito, mantenendo deterministica la navigazione nel sorgente.

## Compatibilita' dei letterali esadecimali

nuBASIC conserva la sintassi esadecimale BASIC originale:

```basic
color% = &hFFFFFF
```

Accetta anche letterali esadecimali in stile C:

```basic
color% = 0xFFFFFF
```

Questo non sostituisce la forma BASIC. Serve a rendere piu' semplice copiare
costanti da header C, documentazione Win32, esempi POSIX e codice grafico.

## Testabilita'

Il lavoro interno e' protetto da test di regressione su piu' livelli:

- I test generali del linguaggio girano via CLI in text mode.
- I debugger contract test esercitano il backend machine-interface.
- I graphics debugger contract test verificano che anche le sessioni grafiche
  riportino correttamente breakpoint ed eventi di step.
- I test di input interrompibile coprono `INPUT` e `INPUT$` interattivi.
- I test runtime coprono semantiche di valore come struct, classi, distruttori,
  riferimenti a oggetti e comportamento numerico/stringa.

Questo conta perche' la rappresentazione runtime ottimizzata deve restare
invisibile ai programmi BASIC. I test proteggono il contratto: le prestazioni
possono migliorare, ma il comportamento osservabile del linguaggio deve restare
stabile.

## Perche' migliora il linguaggio

Queste modifiche non sono solo ottimizzazioni. Rendono nuBASIC piu' coerente
come implementazione di linguaggio:

- Il comportamento del debugger appartiene al modello dell'interprete, non a
  workaround della UI.
- I programmi grafici possono essere debuggati con la stessa semantica dei
  programmi testuali.
- Il codice BASIC ricco di scalari evita traffico heap non necessario.
- Struct e oggetti diventano piu' pratici per programmi piu' grandi.
- Gli esempi di API native possono essere scritti in BASIC leggibile invece che
  come manipolazione densa di buffer.
- Lo stesso core dell'interprete puo' servire CLI, IDE nativa, estensione VS
  Code, test e strumenti futuri.

Il risultato e' un runtime che resta piccolo e comprensibile, ma ora e' molto
piu' preparato per workflow moderni e programmi BASIC di dimensioni maggiori.
