/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 21:26:11 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/23 13:03:01 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const int port, const std::string &password) : _port(port), _password(password) {
	//creo socket (fd) (unico scopo: accettare nuove connessioni)
	//socket() alloca un nuovo socket nel kernel e ritorna il file descriptor
	//AF_INET -> socket IPv4
	//SOCK_STREAM -> socket TCP (stream-oriented)
	// protocollo 0 -> TCP (default per SOCK_STREAM)
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0)
		throw std::runtime_error("Error creating socket");
	fcntl(_serverFd, F_SETFL, O_NONBLOCK); // setto socket non-bloccante per accettare connessioni senza bloccare il server
	//setsockopt() con SO_REUSEADDR permette il riuso immediato della porta (latirmenti rimane bloccata per 60 sec)
	int opt = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
		close(_serverFd);
		throw std::runtime_error("Error setting socket options");
	}
	//salvo l'indirizzo del server in una struct sockaddr_in (IPv4)
	struct sockaddr_in server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY; // accetta connessioni su tutte le interfacce
	server_addr.sin_port = htons(_port); // porta in network byte order
	//associo socket - indirizzo locale (ip + port) con bind()
	if (bind(_serverFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		close(_serverFd);
		throw std::runtime_error("Error binding socket");
	}
	//metto il socket in stato listen ()
	if (listen(_serverFd, SOMAXCONN) < 0) {
		close(_serverFd);
		throw std::runtime_error("Error listening on socket");
	}
	_fds.push_back((struct pollfd){_serverFd, POLLIN, 0}); // aggiungo socket server a _fds per monitorare nuove connessioni
	std::cout << "Server started on port " << _port << std::endl; // server pronto ad ascoltare
	//server pronto ad ascoltare
}

Server::~Server() {
	for (size_t i = 0; i < _fds.size(); i++)
		close(_fds[i].fd);
}

/* -------------------------------------------------------------------------- */

//The server must be capable of handling multiple clients simultaneously without hanging.
// -> poll() ascolta eventi sui file descriptor

// Forking is prohibited. All I/O operations must be non-blocking.
// poll() + recv() (in questo ordine)
// recv() legge i dati arrivati su un socket e li mette in un buffer


/*
	poll() e' una funziona bloccante, si ferma finche' non succede un evento
	ma ha un parametro timeout che cambia il suo comportamento:

	timeout = -1 → aspetta all'infinito, ritorna solo quando succede qualcosa
	timeout = 0 → non aspetta per niente, controlla e ritorna subito (non-bloccante ma consuma piu' CPU)
	timeout = 5000 → aspetta massimo 5 secondi, poi ritorna comunque (anche se non accade niente)
*/
/*
	_fds e' un vector di <struct pollfd>.
	Nella struct prima si definisce l'fd da monitorare e l'event da ascoltare.
	poll() mette gli eventi passati in _fd[].revents (anche alcuni non richiesti)
	I tipi di evento sono:
	POLLIN → ci sono dati da leggere (o una nuova connessione sul server socket)
	POLLHUP → il client si è disconnesso
	POLLERR → errore sul fd
	C'e' anche:
	POLLOUT → scrivere senza bloccarti -> serve quando buffer scrittura pieno
*/
void	Server::run(){
	const int timeout = -1; // aspetta eventi all'infinito
	// loop con poll() che aspetta eventi sugli fd aperti (multiplexing)
	while (true)
	{
		poll(&_fds[0], _fds.size(), timeout);
		// qui poll e' ritornato perche' e' successo qualcosa

		for (size_t i = 0; i < _fds.size(); i++)
		{
			// controllo revents per ogni fd
		}
	}
	// se fd nuovo client + POLLIN (ci sono dati da leggere)
		// handshake (PASS, NICK, USER)
		// -> accept(), crea nuovo socket decato a questo client
		// nuova istanza di Client che salvo in una struttura in Server?
		//se fd è gia client + POLLIN
			//-> recv()
			//-> estraggo mesaggio completo
			//-> gestisco messaggio
		// gestione errore / disconnessione
			// disconnessione + rimuover client dalla struttra in server
			// chiudere socket
			// togliere client dai canali
}

/* -------------------------------------------------------------------------- */

// porte 0 - 1023 well-known ports / porte privilegiate, richiedono privilegi di root
// le porte sono numeri a 16 bit, quindi il max è 2¹⁶ - 1
bool	Server::isValidPort(const std::string &port) {
	if (port.empty() || port.size() > 5) // check per rischio overflow con atoi, max 65535 = 5 cifre
		return (false);
	for (size_t i = 0; i < port.size(); i++) {
		if (!std::isdigit(port[i]))
			return (false);
	}
	int port_num = std::atoi(port.c_str());
	return (port_num > 1024 && port_num <= 65535);
}

// criteri per password??
// no spazi? rischio di troncamento da parte del server quando client manda messaggi
// (vanno gestiti con buffer)
bool	Server::isValidPassword(const std::string &password) {
	if (password.empty())
		return (false);
	for (size_t i = 0; i < password.size(); i++) {
		if (std::isspace(password[i]))
			return (false);
	}
	return (true);
}
