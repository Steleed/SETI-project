# IPortbook - Social Network Client/Server System

## 📋 Overview

IPortbook è un sistema di social network implementato in C che consente a più client di connettersi a un server centrale per stabilire relazioni di amicizia e scambiarsi messaggi. Il progetto implementa un protocollo di comunicazione basato su TCP (per messaggi) e UDP (per notifiche in tempo reale).

### Caratteristiche principali
- **Registrazione e autenticazione** degli utenti
- **Richieste di amicizia** bidirezionali
- **Scambio messaggi privati** tra amici
- **Messaggi di flooding** (broadcast agli amici e amici degli amici)
- **Notifiche in tempo reale** via UDP
- **Lista utenti online** e registrati
- **Sistema di flussi** per messaggi offline

---

## 🏗️ Architettura del Progetto

### Struttura dei file

```
SETI-project/
├── client.c                 # Main del client
├── client.h                 # Header del client
├── client_aux.c             # Funzioni ausiliarie del client
├── server.c                 # Main del server
├── server.h                 # Header del server
├── server_aux.c             # Funzioni ausiliarie del server
├── project.h                # Costanti e definizioni condivise
├── ip.txt                   # Indirizzo IP del server
├── logClient.txt            # Log delle operazioni del client
└── README.md                # Questo file
```

### Componenti principali

#### Client
- **`client.c`**: Punto di ingresso, gestione del menu principale e ciclo di comunicazione
- **`client_aux.c`**: Funzioni per socket, parsing argomenti, building messaggi TCP/UDP
- **Thread UDP**: Ascolto background su porta UDP per notifiche dal server

#### Server
- **`server.c`**: Punto di ingresso, accettazione connessioni, creazione thread per client
- **`server_aux.c`**: Logica di business (registrazione, amicizia, messaggi, flooding)
- **Thread per client**: Ogni client connesso ha un thread dedicato per gestire le richieste

---

## 🔧 Compilazione

### Prerequisiti
- GCC (GNU C Compiler)
- Linux (il progetto usa librerie POSIX)

### Compilazione manuale

**Client:**
```bash
gcc -o client client.c client_aux.c -lpthread
```

**Server:**
```bash
gcc -o server server.c server_aux.c -lpthread
```

### Esecuzione

**1. Avviare il server (su una porta TCP):**
```bash
./server
```
Il server si mette in ascolto sulla porta TCP 6769 e crea dinamicamente un socket UDP per inviare notifiche.

**2. Configurare l'IP del server:**
Creare un file `ip.txt` contenente l'indirizzo IP del server:
```
127.0.0.1 per farlo girare in localhost usando più terminali
```

**3. Avviare il/i client:**
```bash
./client -i IDclient -p portaUDP
```
ID e porta devono essere lunghi rispattivamente 8 e 4 caratteri

Esempio:
```bash
./client -i alice123 -p 5000
./client -i bob12345 -p 5001
```

---

## 📡 Protocollo di Comunicazione

### Messaggi TCP

Tutti i messaggi TCP hanno il formato: `[HEADER argomenti +++]` dove `+++` è il terminatore.

#### Messaggi supportati

| Header | Tipo | Descrizione |
|--------|------|-------------|
| REGIS | Richiesta | Registrazione nuovo utente |
| CONNE | Richiesta | Connessione utente registrato |
| FRIE? | Richiesta | Richiesta di amicizia |
| MESS? | Richiesta | Invio messaggio privato |
| FLOO? | Richiesta | Invio messaggio flooding |
| LIST? | Richiesta | Richiesta lista utenti |
| CONSU | Richiesta | Consultazione flussi/notifiche |
| IQUIT | Richiesta | Disconnessione |
| WELCO | Risposta | Registrazione accettata |
| GOBYE | Risposta | Rifiuto o disconnessione |
| HELLO | Risposta | Connessione accettata |

### Messaggi UDP

Formato: `[YXX]` (3 caratteri)
- **Y**: Codice notifica (0=richiesta amicizia, 1=accettazione, 2=rifiuto, 3=messaggio, 4=flooding)
- **XX**: Numero flussi non consultati (codifica little-endian esadecimale)

Esempio: `[310]` = 3 messaggi da leggere

---

## 🔐 Strutture Dati Principali

### Client (`client_id`)
```c
typedef struct {
    char ID[9];                    // ID univoco (8 caratteri)
    char PORT[5];                  // Porta UDP (4 cifre)
    uint16_t MDP;                  // Password (0-65535)
    int fdTCP;                     // Socket TCP
    int fdUDP;                     // Socket UDP
    u_int16_t num_notifications;   // Contatore notifiche
    int messLength;                // Lunghezza messaggio
    pthread_mutex_t mtx;           // Mutex per sincronizzazione
    FILE *log;                     // File log operazioni
} client_id;
```

### Server (`Client`)
```c
typedef struct {
    char ID[9];                    // ID univoco
    uint16_t password;             // Password
    char udpPort[5];               // Porta UDP client
    struct sockaddr_in clientAddr; // Indirizzo client
    int socketTCP;                 // Socket TCP (o -1 se offline)
    Friends* friends;              // Lista amici (linked list)
    int friendsCount;              // Numero amici
    FluxNode *pendingFluxes;       // Coda messaggi in attesa
    u_int16_t pendingCount;        // Numero messaggi in coda
    pthread_mutex_t userMutex;     // Mutex per sincronizzazione
} Client;
```

---

## 🔄 Flusso Operativo Principale

### 1. Avvio Client
```
Client → Connessione TCP al server
       → Scelta: [1] Registrazione, [2] Connessione, [3] Esci
       → Avvio thread UDP in background per notifiche
```

### 2. Registrazione
```
Client: "REGIS ID PORTA PASSWORD+++"
Server: Verifica ID univoco e disponibilità spazi
      → "WELCO+++" (successo) o "GOBYE+++" (fallimento)
```

### 3. Scambio Messaggi
```
Client A: "FRIE? bob12345+++" (richiesta amicizia)
   ↓
Server: Verifica esistenza bob12345
      → Invia notifica UDP a Bob: "010"
      → Aggiunge flusso alla coda di Bob
   ↓
Bob (consultando): Riceve richiesta amicizia
                 → Accetta: "OKIRF+++"
                 → Server registra amicizia bidirezionale
                 → Notifica Alice: "110"
```

### 4. Messaggi Offline
```
Quando Alice invia messaggio a Bob offline:
  → Messaggio salvato in "flussi pendenti" di Bob
  → Notifica UDP inviata (Bob la riceve al riavvio UDP listener)
  → Al login di Bob, può consultare il messaggio con CONSU
```

---

## 🧵 Gestione Concorrenza

### Mutex
- **`usersListMutex`**: Protegge accesso all'array globale `users[]`
- **`userMutex` (per utente)**: Protegge lista amici e coda flussi di ogni utente

### Thread
- **Server**: Un thread per ogni client connesso
- **Client**: Un thread background per ascoltare notifiche UDP

### Sincronia
```c
pthread_mutex_lock(&usersListMutex);
// Accesso critico alla lista utenti
pthread_mutex_unlock(&usersListMutex);
```

---

## 📝 Specifiche Tecniche

### Limiti del Sistema
- **Max client**: 100 per server
- **Lunghezza esatta ID**: 8 caratteri alfanumerici
- **Max lunghezza messaggio**: 200 caratteri
- **Password**: 0 - 65535 (16 bit)
- **Porta UDP**: 0 - 9999

### Codifica Password
La password è inviata in **little-endian** su 2 byte separati:
```c
byte1 = password & 0xFF;           // Byte meno significativo
byte2 = (password >> 8) & 0xFF;    // Byte più significativo
```

### Log
- **Client**: Scrive in `logClient.txt` tutti i messaggi ricevuti
- **Server**: Stampa in stdout i log di debug

---

## 🐛 Gestione Errori

### Client
- Verifica lunghezza ID (8 caratteri)
- Verifica range porta (0-9999)
- Verifica range password (0-65535)
- Timeout connessione e lettura dati
- Chiusura corretta socket e file

### Server
- Rifiuta duplicati di ID
- Limita max 100 client
- Valida formato messaggi
- Sincronizza accesso a dati condivisi
- Rileva disconnessioni cliente

---

## 🎮 Esempio di Utilizzo Completo

### Terminal 1 - Server
```bash
$ ./server
[LOG] Listening on port 6769...
[LOG] nuova connessione (127.0.0.1)
[PARSER] messaggio REGIS ricevuto
[REGIS] Utente alice123 registrato con successo (Indice: 0)
```

### Terminal 2 - Client Alice
```bash
$ ./client -i alice123 -p 5000
Inserisci la password: 1234
Benvenuto su IPortbook!
1. Registrazione
2. Connessione
3. Esci
> 1
Registrazione avvenuta con successo!

Scegli un'opzione:
1. Richiedi amicizia
2. Manda messaggio
3. Manda flood
4. Visualizza elenco utenti
5. Consulta le notifiche
6. Disconnettiti
> 4
Lettura lista di 1 utenti:
alice123
```

### Terminal 3 - Client Bob
```bash
$ ./client -i bob12345 -p 5001
Inserisci la password: 5678
Benvenuto su IPortbook!
> 1
Registrazione avvenuta con successo!

> 4
Lettura lista di 2 utenti:
alice123
bob12345

> 1
Inserisci l'ID per la richiesta d'amicizia: alice123
Richiesta d'amicizia inviata con successo!
Nuova richiesta d'amicizia (UDP notification ricevuta)
```

---

## ⚠️ Note Importanti

1. **IP del server**: Configurare `ip.txt` con l'IP corretto (127.0.0.1 per localhost)
2. **Porte UDP**: Assicurarsi che le porte non siano già in uso
3. **Numero massimo client**: 100 per server (modificabile in `project.h`)
4. **Messaggi offline**: Salvati fino a consultazione, non persistenti tra riavvii server
5. **Thread safety**: Tutti gli accessi ai dati condivisi sono protetti da mutex

---

## 📚 Riferimenti Protocollo

Consultare il file di specifica del progetto (PDF fornito) per:
- Formato dettagliato di ogni messaggio
- Codifiche little-endian
- Scenari di comunicazione completi
- Diagrammi delle transizioni di stato

---

## 👥 Autori
Patrizio Giordano
Davide Sinagra

Progetto SETI 2025/2026 - Università di Genova