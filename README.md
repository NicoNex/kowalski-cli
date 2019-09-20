# Kowalski

## Analisi

### Descrizione del sistema
Programma per la gestione di trasporto collettivo.

Il programma simula una parte delle funzionalità di una piattaforma per la prenotazione di trasporto collettivo.

La piattaforma consente a chi offre un passaggio di poter inserire la disponibilità di posti, verso una determinata destinazione in una specifica data. Chi cerca un passaggio per una determinata destinazione indica per quanti passeggeri cerca il passaggio, da dove intende partire e quando.

Nello specifico Kowalski si occupa di gestire le prenotazioni, aggiunte, modifiche e cancellazione dei passaggi e guidatori nel sistema.

Il comportamento di kowalski è definito dal codice in `main.c`.
Nel `main` infatti è presente tutta la logica che consente a kowalski di gestire l'I/O con l'utente implementata come un automa a stati finiti.

Gli stati possibili dell'automa coincidono con le scelte intraprese dall'utente e sono definiti nell'enum `choices` presente in `main.c`.

Per il corretto funzionamento di Kowalski sono stati implementati i moduli `drivers.c` e `travels.c` definiti rispettivamente in `include/drivers.h` e `include/travels.h`.
Questi moduli si occupano di caricare da disco, aggiornare e gestire la lista di guidatori e passaggi.

Tali liste sono salvate sul disco come file `json` in `res/drivers.json` e `res/travels.json`.

Durante l'esecuzione del programma tali file vengono costantemente aggiornati con i valori presenti in memoria, con politiche di write-through.

In questi moduli sono anche presenti tutte le funzioni base per la rimozione, aggiunta e modifica dei rispettivi oggetti (`struct driver` e `struct travel`) nelle linked list usate.

Il modulo `list.c` definito in `include/list.h` contiene tutte le funzioni e procedure necessare per una corretta gestione della lista.
Il tipo definito in `include/list.h` chiamato `list_t` infatti è fondamentale per la gestione di tutte le liste presenti nel programma.


I guidatori in drivers.c sono contraddistinti da un token univoco rappresentato da un numero intero a 64 bit.
Tale numero rappresenta lo UNIX Epoch time, quindi il numero di secondi trascorsi dal 1 gennaio 1970 ore 00:00, assegnato automaticamente in fase di aggiunta del guidatore all'elenco, viene usato anche per collegare i passaggi ai rispettivi guidatori.

Infine il modulo `filehandler.c` definito in `include/filehandler.h` racchiude la funzione responsabile del caricamento dei json contenenti le informazioni su guidatori e passaggi dal disco in memoria.


### Requisiti funzionali

| Codice | Nome                    | Descrizione                                                                  |
|--------|-------------------------|------------------------------------------------------------------------------|
| R01    | Guidatori               | Elenco di tutti i guidatori.                                                 |
| R02    | Passaggi                | Elenco di tutti i passaggi.                                                  |
| R03    | I/O                     | Gestione dell'I/O e interpretazione dello stesso.                            |
| R04    | Strutturazione dei dati | Strutturazione dei dati in maniera funzionale alle specifiche del programma. |


### Strumenti di sviluppo
Il programma è stato realizzato su ambiente GNU/Linux con CPU Intel i7 8700K di ottava generazione, architettura x86_64 e 16GB di RAM.

L'ambiente di sviluppo adottato è stato Sublime Text 3 come editor di testo semplice, e gcc come compilatore versione 9.1.0.

Per compilare il sistema è necessario innanzitutto di clonare questo repo con
```sh
git clone https://github.com/NicoNex/kowalski-cli
```

Successivamente verificare che la dipendenza [json-c](https://github.com/json-c/json-c) sia presente nel sistema, altrimenti installarla.

Per installarla su **Ubuntu** e derivate basterà il comando:
```sh
sudo apt install libjson-c-dev
```

Per installarla su MacOS basta aprire un terminale e digitare i seguenti comandi **senza** il simbolo del dollaro $:
```sh
$ git clone https://github.com/json-c/json-c.git
$ cd json-c
$ sh autogen.sh
```
seguiti da
```sh
$ ./configure
$ make
$ make install
```

Una volta installato json-c basterà digitare nel terminale `make` trovandosi nella root del progetto. Verrà così generato il binario `kowalski` che potrà essere eseguito.

## Progettazione

### Progettazione dei tipi e strutture dati
| Nome                | Tipologia             | Descrizione                                                     | Campi                                                                                                                                                             |
|---------------------|-----------------------|-----------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| choices             | enum                  | Descrive la scelta optata dall'utente.                          | QUIT, PRINT_DRIVERS, PRINT_TRAVELS, RATE_DRIVER, BEST_DRIVERS, SEARCH_TRAVEL, BOOK_TRAVEL, ADD_DRIVER, MOD_DRIVER, DEL_DRIVER, ADD_TRAVEL, MOD_TRAVEL, DEL_TRAVEL |
| search_travel_modes | enum                  | Contiene le modalità di ordinamento dei   passaggi cercati.     | BY_PRICE, BY_RATING                                                                                                                                               |
| list_t              | typedef struct node * | Il tipo list_t rappresenta il puntatore alla testa della lista. | struct node *                                                                                                                                                     |
| node                | struct                | Il tipo che rappresenta un nodo in una lista puntata.           | ptr: void *, next: struct node *                                                                                                                                  |
| driver              | struct                | Il tipo che rappresenta un guidatore.                           | id: int, age: int,   rating: int, name: char *,   vehicle: char *, token: long                                                                                    |
| travel              | struct                | Il tipo che rappresenta un passaggio.                           | id: int, date: char *, price: float, destination: char *, seats: int, token: long                                                                                 |
| json_object         | struct                | Il tipo che rappresenta un json.                                | I campi sono definiti nella documentazione della libreria json-c.                                                                                                 |
### Progettazione di librerie e funzioni
Le librerie progettate e implementate sono `drivers.h`, `filehandler.h`, `list.h`, `travels.h`.

Per una documentazione completa fai riferimento a ciascun header file:
- [drivers.h](https://github.com/NicoNex/kowalski/blob/master/include/drivers.h)
- [filehandler.h](https://github.com/NicoNex/kowalski/blob/master/include/filehandler.h)
- [list.h](https://github.com/NicoNex/kowalski/blob/master/include/list.h)
- [travels.h](https://github.com/NicoNex/kowalski/blob/master/include/travels.h)

Per la documentazione di [json-c](https://github.com/json-c/json-c), fare riferimento al seguente link http://json-c.github.io/json-c/ .
