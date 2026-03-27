# TODO — ft_irc

- [X] Testare molto con valgrind!! valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes --error-exitcode=1 ./ircserv 6667 password
- [X] Gestione segnali (SIGINT, SIGTERM) per chiusura pulita del server?

## Fase 0 — Parsing argomenti
- [X] Validazione numero argomenti
- [X] Validazione porta (intervallo 1-65535)
- [X] Validazione password (controlla criteri di sicurezza)

## Fase 1 — Server core
- [X] Classe Server — attributi e costruttore
- [X] Validazione porta e password
- [X] Creazione server socket
- [X] setsockopt SO_REUSEADDR
- [X] bind()
- [X] listen()
- [X] poll() loop base
- [X] accept() — nuova connessione
- [X] recv() — lettura dati
- [X] Buffer per messaggi parziali
- [X] Estrazione messaggi completi (\r\n)
- [X] Disconnessione client (POLLHUP / recv() == 0)
- [X] Classe Client — attributi base
- [X] Parser messaggi IRC (prefix, command, params, trailing)

## Fase 2 — Handshake (tutti insieme)
- [X] PASS
- [X] NICK + check nickname uniqueness
- [X] USER
- [X] Welcome messages (001 002 003 004)
- [X] PING / PONG
- [X] QUIT
- [X] PRIVMSG (privato)

## Fase 3 — Canali
- [X] Classe Channel — attributi base
- [X] JOIN
- [X] PRIVMSG (canale)
- [X] TOPIC (lettura)

## Fase 4 — Operatori
- [.] KICK // wip caro
- [X] INVITE // da modificare i messaggi di errore (per hexchat)
- [X] KICK
- [X] INVITE
- [X] Test con nc — messaggi parziali
- [X] TOPIC (modifica)
- [X] MODE (lettura)
- [ ] MODE +i
- [ ] MODE +t
- [ ] MODE +k
- [ ] MODE +o
- [ ] MODE +l

## Fase 5 — Test
- [X] Test con nc — messaggi parziali
- [X] Test con HexChat — connessione base
- [X] Test con più client contemporanei
- [X] Test disconnessione brusca
- [X] Test tutti i comandi operatore
- [X] Test errori numerici

## Fase 6 — LOGS
- [ ] funzione log normali
- [ ] funzione error log

## TO  FIX:
- [X] NICK: se nick già registrato e cambio nick → notifica il client e tutti i canali in cui si trova
- [X] NICK: aggiungere isValidNickname (esiste già)
- [X] PRIVMSG nel canale non va
- [X] server si spegne: client cosa deve fare??? analizzare comportamento atteso (nc: write failed (0/2): Broken pipe) : sì, è normale che il client riceva un errore di "Broken pipe" quando il server si spegne, poiché la connessione viene chiusa in modo imprevisto. Il client dovrebbe gestire questa situazione in modo appropriato, ad esempio mostrando un messaggio di errore all'utente o tentando di riconnettersi, ma non e' qualcosa che possiamo risolvere dal lato server, è una conseguenza naturale della chiusura della connessione.
- [] (Server) 8.9 Tracking nickname changes -> https://www.rfc-editor.org/rfc/rfc1459.html#section-4.2
- [X] `PING ciao` restituisce `:ircserv PONG ircserv :ciao` e' giusto? : sì, è corretto. Il server dovrebbe rispondere con un messaggio PONG che include il nome del server e il parametro del PING ricevuto.
- [X] puo' esistere username duplicato? -> se username e' gia' in uso non ottengo nessuna risposta dal server : yes, è possibile avere username duplicati, ma non nickname duplicati. Il server dovrebbe rispondere con un messaggio di errore se si tenta di registrare un nickname già in uso, ma non dovrebbe impedire la registrazione di username duplicati.

- [X] dopo il QUIT che succede? (client nc non si chiude) : si chiude la connessione lato server, ma il client potrebbe non chiudersi automaticamente. Il client dovrebbe gestire la chiusura della connessione in modo appropriato, ad esempio mostrando un messaggio di disconnessione all'utente o terminando l'applicazione, ma non è qualcosa che possiamo risolvere dal lato server, è una conseguenza naturale della chiusura della connessione.

PRIVMSG #42: ciao
:ircserv 412 Pau :No text to send //fixxato ora da channel sconosciuto

## TO  IMPROVE:
- [] logging messaggi sul client -> mandare prefix server
- [] logging messaggi sul server -> mandare  info client + messaggi extra (non solo fd)

- [] PARSING DA AGGIUSTARE??
PRIVMSG #42 :ciao
PRIVMSG #42: ciao
:ircserv 403 nick #42: :No such channel