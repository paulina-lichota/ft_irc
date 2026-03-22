# IRC — Comandi

## Connessione al server
Sequenza obbligatoria all'avvio:
```
PASS mypassword
NICK mynickname
USER myusername 0 * :myrealname
```

---

## Comandi base

| Comando | Sintassi | Descrizione |
|---|---|---|
| `PASS` | `PASS <password>` | Autenticazione — primo comando da mandare |
| `NICK` | `NICK <nickname>` | Imposta il nickname |
| `USER` | `USER <user> 0 * :<realname>` | Registra l'utente |
| `QUIT` | `QUIT [:<motivo>]` | Disconnessione pulita |
| `PING` | `PING :<token>` | Keep-alive — il server risponde con PONG |
| `PONG` | `PONG :<token>` | Risposta al PING |

PING e PONG servono per mantenere viva la connessione. HexChat li manda automaticamente in background — se il server non risponde con PONG, HexChat si disconnette.

---

## Comandi di canale

| Comando | Sintassi | Descrizione |
|---|---|---|
| `JOIN` | `JOIN <#canale> [<password>]` | Entra in un canale |
| `PART` | `PART <#canale> [:<motivo>]` | Lascia un canale |
| `PRIVMSG` | `PRIVMSG <dest> :<testo>` | Messaggio a canale o utente |
| `TOPIC` | `TOPIC <#canale> [:<topic>]` | Legge o imposta il topic |

---

## Comandi operatore

| Comando | Sintassi | Descrizione |
|---|---|---|
| `KICK` | `KICK <#canale> <nick> [:<motivo>]` | Espelle un utente dal canale |
| `INVITE` | `INVITE <nick> <#canale>` | Invita un utente nel canale |
| `MODE` | `MODE <#canale> <+/-flag> [param]` | Cambia modalità del canale |

---

## MODE — flags

| Flag | Parametro | Descrizione |
|---|---|---|
| `+i` / `-i` | nessuno | Invite-only — solo invitati possono entrare |
| `+t` / `-t` | nessuno | Solo operatori possono cambiare il topic |
| `+k` / `-k` | password | Imposta/rimuove la password del canale |
| `+o` / `-o` | nick | Dà/toglie i privilegi di operatore |
| `+l` / `-l` | numero | Imposta/rimuove il limite massimo di utenti |

---

## Risposte numeriche

### Welcome
| Numerico | Descrizione |
|---|---|
| `001` | Benvenuto |
| `002` | Info sul server |
| `003` | Data di creazione del server |
| `004` | Info versione |

### Canale
| Numerico | Descrizione |
|---|---|
| `331` | Nessun topic impostato |
| `332` | Topic del canale |
| `341` | Conferma INVITE |
| `353` | Lista utenti nel canale |
| `366` | Fine lista utenti |

### Errori
| Numerico | Nome | Causa |
|---|---|---|
| `401` | ERR_NOSUCHNICK | Nick o canale non esiste |
| `403` | ERR_NOSUCHCHANNEL | Canale non esiste |
| `421` | ERR_UNKNOWNCOMMAND | Comando non riconosciuto |
| `431` | ERR_NONICKNAMEGIVEN | Nessun nickname specificato |
| `432` | ERR_ERRONEUSNICKNAME | Nickname non valido |
| `433` | ERR_NICKNAMEINUSE | Nickname già in uso |
| `441` | ERR_USERNOTINCHANNEL | Utente non è nel canale |
| `442` | ERR_NOTONCHANNEL | Tu non sei nel canale |
| `461` | ERR_NEEDMOREPARAMS | Parametri insufficienti |
| `462` | ERR_ALREADYREGISTRED | Già registrato |
| `464` | ERR_PASSWDMISMATCH | Password errata |
| `471` | ERR_CHANNELISFULL | Canale pieno (mode +l) |
| `473` | ERR_INVITEONLYCHAN | Canale invite-only (mode +i) |
| `475` | ERR_BADCHANNELKEY | Password canale errata (mode +k) |
| `482` | ERR_CHANOPRIVSNEEDED | Non sei operatore del canale |