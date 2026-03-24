# TODO — ft_irc

- [ ] Gestione segnali (SIGINT, SIGTERM) per chiusura pulita del server?

## Fase 0 — Parsing argomenti
- [X] Validazione numero argomenti
- [X] Validazione porta (intervallo 1-65535)
- [x] Validazione password (controlla criteri di sicurezza)

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
- [ ] Parser messaggi IRC (prefix, command, params, trailing)

## Fase 2 — Handshake (tutti insieme)
- [ ] PASS
- [ ] NICK
- [ ] USER
- [ ] Welcome messages (001 002 003 004)
- [ ] PING / PONG
- [ ] QUIT

## Fase 3 — Canali
- [ ] Classe Channel — attributi base
- [ ] JOIN
- [ ] PART
- [ ] PRIVMSG (canale)
- [ ] PRIVMSG (privato)
- [ ] TOPIC (lettura)

## Fase 4 — Operatori
- [ ] KICK
- [ ] INVITE
- [ ] TOPIC (modifica)
- [ ] MODE +i
- [ ] MODE +t
- [ ] MODE +k
- [ ] MODE +o
- [ ] MODE +l

## Fase 5 — Test
- [ ] Test con nc — messaggi parziali
- [ ] Test con HexChat — connessione base
- [ ] Test con più client contemporanei
- [ ] Test disconnessione brusca
- [ ] Test tutti i comandi operatore
- [ ] Test errori numerici
