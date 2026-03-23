# Server — Costruttore

Il costruttore del Server ha un unico scopo: preparare il socket del server in modo che sia pronto ad accettare connessioni dai client. Se uno qualsiasi dei passi fallisce, il costruttore lancia un'eccezione e il server non viene creato.

---

## 1. socket()

```cpp
_serverFd = socket(AF_INET, SOCK_STREAM, 0);
```

`socket()` chiede al kernel di allocare un nuovo socket e ritorna il suo file descriptor — un numero intero che useremo da questo momento in poi per riferirci al socket.

| Parametro | Valore | Significato |
|---|---|---|
| `AF_INET` | address family | IPv4 |
| `SOCK_STREAM` | tipo | connessione affidabile e ordinata (TCP) |
| `0` | protocollo | default per questa combinazione, ovvero TCP |


---

## 2. fcntl()

```cpp
fcntl(_serverFd, F_SETFL, O_NONBLOCK);
```

Rende il socket non-bloccante. Per default un socket è **bloccante** — se chiami `accept()` e non c'è nessun client in attesa, il processo si ferma e aspetta finché non arriva qualcuno, bloccando tutto il server.

Con `O_NONBLOCK` il comportamento cambia — se chiami `accept()` o `recv()` e non c'è niente disponibile, la funzione ritorna immediatamente con `errno = EAGAIN` invece di bloccarsi.

Questo è fondamentale perché il server ha un solo processo che deve gestire più client contemporaneamente. Se una syscall si bloccasse, tutti gli altri client rimarrebbero in attesa.

In pratica, combinato con `poll()`, non ti capiterà quasi mai di chiamare `accept()` o `recv()` senza che ci sia qualcosa da leggere — perché `poll()` ti dice prima che c'è un evento. `O_NONBLOCK` è una rete di sicurezza per i casi edge in cui qualcosa va storto tra il momento in cui `poll()` segnala l'evento e il momento in cui chiami la syscall.

---

## 3. setsockopt()

```cpp
int opt = 1;
setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
```

Configura un'opzione sul socket appena creato. In questo caso abilitiamo `SO_REUSEADDR`.

**Perché serve?** Quando il server si chiude, la porta rimane in stato `TIME_WAIT` per qualche secondo. Senza questa opzione, riavviare il server subito dopo causerebbe un errore `EADDRINUSE` — "porta già in uso" — anche se il server precedente non sta più girando.

**I parametri:**
- `SOL_SOCKET` — il livello a cui si applica l'opzione, ovvero il livello socket. `SO_REUSEADDR` è un'opzione generica del socket, non specifica di TCP o IP
- `SO_REUSEADDR` — l'opzione da impostare
- `&opt` — puntatore al valore. `setsockopt()` prende `const void *` perché è generica — non sa in anticipo che tipo di valore riceverà. Per `SO_REUSEADDR` serve un intero, quindi passiamo l'indirizzo di `opt`, che è un `int` con valore `1` (abilitato)
- `sizeof(int)` — quanti byte leggere a partire da quell'indirizzo, necessario proprio perché il tipo è perso nel `void *`

---

## 4. bind()

```cpp
struct sockaddr_in server_addr;
std::memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = AF_INET;
server_addr.sin_addr.s_addr = INADDR_ANY;
server_addr.sin_port = htons(_port);

bind(_serverFd, (struct sockaddr *)&server_addr, sizeof(server_addr));
```

`bind()` associa il socket a un indirizzo locale — ovvero dice al kernel: "tutti i dati che arrivano su questa porta devono essere consegnati a questo socket".

Prima di chiamare `bind()` dobbiamo costruire una `struct sockaddr_in` con le informazioni del server:

| Campo | Valore | Significato |
|---|---|---|
| `sin_family` | `AF_INET` | IPv4, stesso valore usato in `socket()` |
| `sin_addr.s_addr` | `INADDR_ANY` | ascolta su tutte le interfacce di rete disponibili |
| `sin_port` | `htons(_port)` | porta in network byte order |

**Perché `memset()`?** `sockaddr_in` ha campi di padding (come `sin_zero`) che devono essere azzerati. Senza `memset()` quei byte avrebbero valori casuali dello stack e `bind()` potrebbe comportarsi in modo imprevedibile.

**Perché `INADDR_ANY`?** Se il server ha più interfacce di rete (ad esempio Ethernet e Wi-Fi), `INADDR_ANY` dice al kernel di accettare connessioni su tutte le interfacce. Se volessimo limitare il server a una sola interfaccia, dovremmo specificarne l'indirizzo IP qui.

**Perché `htons()`?** I processori possono memorizzare i byte in ordine diverso (little-endian o big-endian). La rete usa sempre big-endian, detto *network byte order*. `htons()` — *host to network short* — converte la porta dalla rappresentazione del sistema a quella della rete.

**Perché il cast a `struct sockaddr *`?** `bind()` è una funzione generica che funziona sia con IPv4 che IPv6. Per questo accetta `struct sockaddr *` come tipo base. Noi passiamo una `struct sockaddr_in` (specifica IPv4) e la castiamo al tipo generico.


---

## 5. listen()

```cpp
listen(_serverFd, SOMAXCONN);
```

Mette il socket ufficialmente in ascolto. Prima di questa chiamata il socket esiste ed è associato a una porta, ma non accetta ancora connessioni.

Il secondo parametro è il **backlog**.

Per capirlo bisogna sapere che quando un client vuole connettersi, non è `accept()` il primo a sapere — è il **kernel**. Il kernel gestisce autonomamente il TCP handshake a tre vie:
```
client → SYN      → kernel
client ← SYN-ACK ← kernel
client → ACK      → kernel
```

Tutto questo avviene prima che il tuo programma faccia qualsiasi cosa. Quando l'handshake è completato, la connessione è già stabilita a tutti gli effetti — il kernel la mette in una coda interna chiamata accept queue e aspetta che il programma chiami accept() per prenderla.
Il backlog è la dimensione massima di quella coda. Esiste perché potrebbero arrivare più connessioni quasi simultaneamente — il kernel le gestisce tutte subito completando gli handshake, ma il programma le processa una alla volta con accept(). La coda fa da buffer tra i due.
Se la coda è piena e arriva un nuovo client, il kernel scarta il SYN direttamente — il client riceverà un timeout e riproverà.
SOMAXCONN usa il massimo consentito dal sistema operativo — su Linux tipicamente 128.

---

## 6. Inizializzazione di `_pollFds`

```cpp
struct pollfd server_pollfd;
server_pollfd.fd = _serverFd;
server_pollfd.events = POLLIN;
server_pollfd.revents = 0;
_pollFds.push_back(server_pollfd);
```

`_pollFds` è il vettore di `struct pollfd` che `poll()` userà nel loop principale per monitorare tutti i file descriptor aperti — il server socket e tutti i client connessi.

`struct pollfd` ha tre campi:

| Campo | Chi lo scrive | Significato |
|---|---|---|
| `fd` | tu | quale fd monitorare |
| `events` | tu | quali eventi ti interessano |
| `revents` | il kernel | quali eventi si sono verificati |

La distinzione tra `events` e `revents` è importante — `events` lo imposti tu prima di chiamare `poll()`, `revents` lo riempie il kernel quando `poll()` ritorna. Sono separati perché potresti voler monitorare eventi diversi su fd diversi.

`revents` è una **maschera di bit** — può avere più flag attivi contemporaneamente. Per esempio un fd potrebbe avere sia `POLLIN` che `POLLHUP` settati nello stesso momento. Per questo motivo va sempre controllato con `&` e non con `==`:

Il server socket viene aggiunto come primo elemento del vettore con `POLLIN` — così `poll()` ci segnala quando un nuovo client vuole connettersi. Da quel momento in poi, ogni volta che `accept()` crea un nuovo client socket, aggiungeremo un nuovo elemento a `_pollFds` con lo stesso pattern. Quando un client si disconnette, rimuoveremo il suo elemento dal vettore.

---

# Server — Loop principale

Il loop principale del server è un ciclo infinito che chiama `poll()` per aspettare eventi su tutti i file descriptor aperti. Quando `poll()` ritorna, controlla quali fd hanno eventi attivi e agisce di conseguenza.
