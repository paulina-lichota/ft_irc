# IRC — Teoria

## Cos'è IRC
IRC (Internet Relay Chat) è un protocollo di comunicazione testuale. Definisce un insieme di comandi, risposte numeriche e regole di formattazione dei messaggi che governano la comunicazione tra client e server.

---

## Server
Un server è un processo in esecuzione continua che accetta connessioni da client, elabora le loro richieste e restituisce risposte.

---

## Client
Un client è un processo che si connette ad un server. Nel contesto IRC, è il programma che l'utente usa per connettersi al server, inviare messaggi e ricevere risposte. Comunica tramite una connessione TCP persistente utilizzando il protocollo IRC.

Ogni utente corrisponde ad un client. Se il nickname è già in uso, il server rifiuta la connessione con `433 ERR_NICKNAMEINUSE`.

### Attributi di un client
- `_nickname` → da NICK
- `_username` → da USER (primo parametro)
- `_realname` → da USER (trailing, dopo i :)
- `_hostname` → IP del client, preso da accept()

---

## Connessione al server
Quando un client si connette deve mandare questi tre comandi nell'ordine:
```
PASS mypassword
NICK mynickname
USER myusername 0 * :myrealname
```
Solo dopo il server risponde con i welcome messages `001 002 003 004`.

## Canali
Un canale è un luogo virtuale dove più utenti possono comunicare tra loro. Viene creato automaticamente quando il primo utente fa JOIN — quel primo utente diventa automaticamente operatore.

Un canale è identificato da un nome che inizia con `#`, con lunghezza massima di 200 caratteri. Non può contenere spazi, virgole o caratteri di controllo.

```
JOIN #channel   → entra nel canale
PART #channel   → lascia il canale
```

Un canale muore quando l'ultimo utente lo lascia.

### Operatori di canale
Sono utenti con privilegi speciali nel canale. Possono eseguire:
- `KICK` — espellere un utente
- `INVITE` — invitare un utente
- `TOPIC` — cambiare il topic
- `MODE` — cambiare le modalità del canale

---

## Formato messaggi
Ogni messaggio IRC segue questo formato:
```
[:prefix] COMANDO [parametri] [:testo]\r\n
```
- **prefix** (opzionale) — identifica chi manda il messaggio: `nick!user@host`
- **COMANDO** — es. NICK, JOIN, PRIVMSG
- **parametri** — separati da spazio
- **testo** — ultimo parametro, può contenere spazi, preceduto da `:`
- Ogni messaggio termina obbligatoriamente con `\r\n`

### Fine messaggio — \r\n
Ogni messaggio IRC termina con due caratteri invisibili:
- `\r` (carriage return, ASCII 13)
- `\n` (newline, ASCII 10)

Quando HexChat manda `NICK pippo`, in realtà manda:
```
NICK pippo\r\n
```

Il server usa `\r\n` per capire dove finisce un messaggio e dove inizia il prossimo:
```
"NICK pippo\r\nJOIN #generale\r\n"
              ↑                 ↑
         fine msg 1        fine msg 2
```

### Risposte numeriche
Il server risponde ai comandi con codici numerici. Esempi:
- `001` → benvenuto
- `433` → nickname già in uso
- `482` → non sei operatore del canale

---

## TCP e Socket

### Cos'è TCP/IP
TCP/IP è il sistema che permette a due computer di parlarsi in rete. È diviso in due livelli:
- **IP** — trova il percorso giusto tra i computer in rete
- **TCP** — garantisce che i dati arrivino nell'ordine giusto e senza perdite

IRC usa TCP perché i messaggi devono arrivare tutti, nell'ordine corretto.

### Cos'è un socket
Un socket è un file descriptor che rappresenta un endpoint di comunicazione di rete, cioè un punto di connessione tra due programmi in rete. In C++ è semplicemente un numero intero — puoi leggerci con `recv()` e scriverci con `send()` come se fosse un file.

```cpp
int fd = socket(AF_INET, SOCK_STREAM, 0);
```

Esistono due tipi di socket nel server:
- **server socket** — resta sempre in ascolto sulla porta, il suo unico scopo è accettare nuove connessioni. Ne esiste uno solo.
- **client socket** — creato da `accept()` per ogni client che si connette. Uno per ogni client.

### Cos'è una porta
Un computer ha tanti programmi in esecuzione contemporaneamente. La porta è un numero che identifica quale programma deve ricevere i dati in arrivo.
```
IP      → quale computer
porta   → quale programma su quel computer
```
Le porte 0-1023 sono riservate al sistema operativo e richiedono privilegi root. La porta viene passata come argomento al server — per convenzione IRC usa 6667, ma qualsiasi porta valida funziona.

### poll()
`poll()` monitora una lista di file descriptor e aspetta che almeno uno abbia qualcosa da fare. È il cuore del server — permette di gestire molti client contemporaneamente con un solo processo.
```
poll() aspetta...
→ server_fd ha POLLIN  → nuovo client → accept()
→ client_fd ha POLLIN  → dati in arrivo → recv()
→ client_fd ha POLLHUP → client disconnesso → chiudi fd
```
Senza `poll()` il server dovrebbe controllare ogni client uno per uno — inefficiente e bloccante.

### recv()
Legge i dati arrivati su un socket e li mette in un buffer. Ritorna:
- `> 0` — numero di byte letti
- `0` — il client ha chiuso la connessione
- `-1` — errore

Va chiamato **solo dopo** che `poll()` ha segnalato `POLLIN` su quel fd — così non si blocca mai.

### send()
Manda dati su un socket. Attenzione — potrebbe mandare meno byte di quelli richiesti. Controlla sempre il valore di ritorno e rimanda il resto se necessario.

### Messaggi parziali — il buffer
TCP non garantisce che un messaggio arrivi intero. `recv()` potrebbe restituire:
```
prima chiamata:  "NICK pip"
seconda chiamata: "po\r\n"
```
Per questo ogni client ha un buffer di ricezione — accumula i byte finché non trova `\r\n`, solo allora il messaggio è completo e può essere processato.

### I/O non-bloccante
Per default un socket è bloccante — `recv()` aspetta finché non arrivano dati, bloccando tutto il server. Con `fcntl()` si rende non-bloccante: `recv()` ritorna subito con `errno = EAGAIN` se non ci sono dati.
```
fcntl(fd, F_SETFL, O_NONBLOCK);
```
Combinato con `poll()`, garantisce che il server non si blocchi mai.

---

## Setup del server — sequenza obbligatoria
```
1. socket()      crea il socket, ritorna un fd
2. setsockopt()  configura SO_REUSEADDR
3. bind()        associa il socket a IP + porta
4. listen()      mette il socket in ascolto
5. poll()        aspetta eventi sugli fd aperti
6. accept()      accetta un nuovo client, crea client_fd
```
Ogni passo dipende dal precedente — l'ordine è obbligatorio.

### setsockopt() e SO_REUSEADDR
Quando il server si chiude, la porta rimane in stato `TIME_WAIT` per qualche secondo. Senza `SO_REUSEADDR`, riavviare il server subito dopo causa `EADDRINUSE` — "porta già in uso".

### bind()
Associa il socket a una porta specifica. Dopo `bind()` il sistema operativo sa che tutti i dati in arrivo sulla porta scelta devono essere consegnati al tuo server.

### listen()
Apre ufficialmente il server al pubblico. Prima di questa chiamata il socket esiste ma non accetta connessioni.

### accept()
Quando `poll()` segnala `POLLIN` sul server socket, significa che un nuovo client vuole connettersi. `accept()` crea un nuovo socket dedicato solo a quel client e ritorna il suo fd.

---

## Testare con nc
```bash
nc -C 127.0.0.1 6667
```
Il flag `-C` è fondamentale — dice a `nc` di mandare `\r\n` quando premi Invio, esattamente come farebbe HexChat. Senza `-C`, `nc` manderebbe solo `\n`.