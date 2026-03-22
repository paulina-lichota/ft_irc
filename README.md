# ft_irc
IRC server written in C++98
- multiple clients handling (no server-to-server communication)
- I/O multiplexing (single thread)
- TCP/IP

## What is IRC?
IRC (Internet Relay Chat) is a protocol for chatting over the Internet, created in 1988.

Clients connect to a server via TCP, send commands (such as JOIN #channel), and the server relays messages.
All communication is plain text terminated by \r\n.

> ✨ Check [Sockets For Dummies](SocketsForDummies.md) for a better understanding of how data travels across the network ✨

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

## Architecture
The server is a single threaded application that listens for incoming connections on a given port, using **poll()** to wait for events on the socket.

When a client connects, the server creates a new socket and associates it with the client.

The server then sends a handshake to the client, which consists of a PASS command followed by a NICK and USER command.
After the handshake, the server waits for messages from the client.
If the client sends a PING command, the server responds with a PONG command.
If the client sends a JOIN command, the server creates a new channel and associates it with the client.
If the client sends a PART command, the server removes the channel from the client's list of channels.
If the client sends a QUIT command, the server closes the connection to the client.

# TODO
- setup server: 
