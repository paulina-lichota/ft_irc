# ft_irc
IRC server written in C++98
- multiple clients handling (no server-to-server communication)
- I/O multiplexing (single thread)
- TCP/IP

## What is IRC?
IRC (Internet Relay Chat) is a protocol for chatting over the Internet, created in 1988.

Clients connect to a server via TCP, send commands (such as JOIN #channel), and the server relays messages.
All communication is plain text terminated by \r\n.

> ✨ Check [Sockets For Dummies](docs/SocketsForDummies.md) for a better understanding of how data travels across the network ✨

## Run
Compile
```bash
make
```

Run
```bash
./ircserv <port> <password>
```

> tradizionalmente si usa la porta 6667

Run client
```bash
nc -C 127.0.0.1 6667
```

## Connection from client
```bash
PASS mypassword
NICK mynickname
USER myusername myhost * : myrealname
```

## Messages
```bash
PING <server>
PONG <server>
```

## Commands
```bash
USER <username> <hostname> * : <realname>
NICK <nickname>
JOIN <channel>
PART <channel>
QUIT [: <quit message>]
```

## Reference
Abbiamo scelto la versione rfc1459 [https://www.rfc-editor.org/rfc/rfc1459.html] per semplicita'.

Differenze con altre versioni:

RFC 2812 (la piu' moderna)
- Formato del comando USER leggermente diverso
- Regole più precise sui nickname validi
- Codici di errore aggiuntivi
- Più dettagli su edge case

RFC 2813:
- riguarda la comunicazione server-to-server. Non ti serve per ft_irc (hai un solo server).

RFC 7194 (2014):
- aggiunge TLS/SSL. Non ti serve.