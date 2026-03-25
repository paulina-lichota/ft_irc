# HexChat

HexChat è un client IRC grafico molto popolare, disponibile su Windows, macOS e Linux. È un ottimo strumento per testare il tuo server IRC durante lo sviluppo, grazie alla sua interfaccia user-friendly e al supporto completo dei comandi IRC.

---

## Installazione

```bash
sudo apt install hexchat   # Ubuntu/Debian
```

---

## Configurazione per ft_irc

1. Apri HexChat
2. Vai su **HexChat → Network List** (oppure premi `Ctrl+S`)
3. Clicca **Add** e dai un nome alla rete, es. `ft_irc`
4. Clicca **Edit** sulla rete appena creata
5. Nel campo **Servers**, sostituisci il testo con l'indirizzo del tuo server, ad esempio:
   ```
   127.0.0.1/6667
   ```
6. Vai al tab **General** e imposta:
   - **Nick name**: il nickname che vuoi usare
   - **User name**: il tuo username
   - **Real name**: il tuo realname
7. Spunta **Use a password** e inserisci la password del tuo server (quella passata come argomento a `./ircserv`)
8. **Deseleziona** "Use SSL" se selezionato perché il server non supporta SSL
9. Clicca **Close** e poi **Connect**

---

## Cosa manda HexChat alla connessione

Quando si connette, HexChat manda automaticamente nell'ordine:

```
PASS <password>
NICK <nickname>
USER <username> 0 * :<realname>
```

Il server deve rispondere con i welcome numerics `001 002 003 004` — solo allora HexChat considera la connessione stabilita e mostra l'interfaccia.

---

## Comandi utili da testare

Una volta connesso, dalla barra di testo in basso:

```
/join #canale          → entra in un canale
/part #canale          → lascia un canale
/msg nickname testo    → messaggio privato
/quit                  → disconnessione
/ping ircserv          → manda un PING al server
```

---

## Troubleshooting

**HexChat si disconnette subito**
→ Il server non sta mandando i welcome numerics `001 002 003 004` dopo l'handshake

**HexChat non si connette**
→ Controlla che il server sia in ascolto sulla porta giusta con `./ircserv 6667 <password>`

**"Bad password"**
→ La password inserita in HexChat non corrisponde a quella passata al server

**HexChat si riconnette in loop**
→ Il server sta chiudendo la connessione — controlla `handlePass` e la gestione degli errori

---

## Test alternativo con nc

```bash
nc -C 127.0.0.1 6667
```

Poi digita manualmente:
```
PASS mypassword
NICK mynick
USER myuser 0 * :My Real Name
```