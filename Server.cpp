/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: francema <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2026/03/24 17:14:32 by francema         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const int port, const std::string &password) : _port(port), _password(password) {
	_serverFd = socket(AF_INET, SOCK_STREAM, 0); //AF_INET -> socket IPv4, SOCK_STREAM -> socket TCP (stream-oriented), protocollo 0 -> TCP (default per SOCK_STREAM)
	if (_serverFd < 0)
		throw std::runtime_error("Error creating socket");
	fcntl(_serverFd, F_SETFL, O_NONBLOCK); // setto socket non-bloccante per accettare connessioni senza bloccare il server
	int opt = 1; //tipo booleano che serve per setsockopt, 1 per attivare SO_REUSEADDR
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) { //SO_REUSEADDR permette il riuso immediato della porta
		close(_serverFd);
		throw std::runtime_error("Error setting socket options");
	}
	struct sockaddr_in server_addr; //salvo l'indirizzo del server in una struct sockaddr_in (IPv4)
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET; // IPv4
	server_addr.sin_addr.s_addr = INADDR_ANY; // accetta connessioni su tutte le interfacce, es. ethernet, wifi, localhost
	server_addr.sin_port = htons(_port); // porta in network byte order
	if (bind(_serverFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) { //associo socket - indirizzo locale (ip + port) con bind()
		close(_serverFd);
		throw std::runtime_error("Error binding socket");
	}
	if (listen(_serverFd, SOMAXCONN) < 0) { //metto il socket in stato listen (), SOMAXCONN è il backlog massimo supportato dal sistema operativo (numero di connessioni in attesa)
		close(_serverFd);
		throw std::runtime_error("Error listening on socket");
	}
	addPollFd(_serverFd); // aggiungo socket del server a _pollFds per monitorarlo con poll() per nuove connessioni
	std::cout << "Server started on port " << _port << std::endl; // server pronto ad ascoltare
}

Server::~Server() {
	for (size_t i = 0; i < _pollFds.size(); i++)
		close(_pollFds[i].fd);
}

/* ------------------------------------ Main run loop ----------------------------------- */

void	Server::run(){
	while (true) {
		int ret = poll(&_pollFds[0], _pollFds.size(), POLL_TIMEOUT);
		if (ret < 0) {
			if (errno == EINTR) // interrotto da un segnale, possiamo ignorare e continuare
				continue;
			std::cerr << "Error in poll()" << std::endl;
			break;
		}
		if (_pollFds[0].revents & POLLIN) //revents è un bitmask, controllo se c'è POLLIN sul server socket (nuova connessione)
			handleNewConnection();
		for (size_t i = 1; i < _pollFds.size(); i++) {
			if (_pollFds[i].revents & POLLIN) {
				if (!handleClientMessage(i)) // se ritorna false -> client disconnesso
					i--; // se client disconnesso, handleClientMessage() chiama handleClientDisconnection() che rimuove il client da _pollFds
			}
			else if (_pollFds[i].revents & (POLLHUP | POLLERR)) { //POLLHUP -> client si è disconnesso, POLLERR -> errore sul fd
				handleClientDisconnection(i);
				i--;
				//tolgo client dai canali
			}
		}
	}
}

/* ------------------------------------ Handlers ----------------------------------- */

// accetto nuova connessione, ottengo nuovo client socket
// setto client socket non-bloccante
// aggiungo client socket a _pollFds per monitorarlo
// creo oggetto Client associato al client socket, lo salvo in _clients con fd come chiave
void Server::handleNewConnection() {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientFd = accept(_serverFd, (struct sockaddr *)&clientAddr, &clientAddrLen); // accetto nuova connessione, ottengo nuovo client socket
	if (clientFd < 0) {
		std::cerr << "Error accepting new connection" << std::endl;
		return ;
	}
	fcntl(clientFd, F_SETFL, O_NONBLOCK); // setto client socket non-bloccante
	addPollFd(clientFd); // aggiungo client socket a _pollFds per monitorarlo
	std::string hostname = inet_ntoa(clientAddr.sin_addr); // ottengo hostname del client da indirizzo IP
	_clients[clientFd] = Client(clientFd, hostname); // creo oggetto Client associato al client socket, lo salvo in _clients con fd come chiave
	std::cout << "New client connected: fd " << clientFd << std::endl;
}

void Server::handleClientDisconnection(size_t index) {
	std::cout << "Client fd " << _pollFds[index].fd << " disconnected" << std::endl;
	close(_pollFds[index].fd);
	_clients.erase(_pollFds[index].fd);
	_pollFds.erase(_pollFds.begin() + index);
}

bool Server::handleClientMessage(size_t index) {
	char buffer[IRC_MSG_MAX_LEN]; // buffer temporaneo per leggere dati da client socket
	int n = recv(_pollFds[index].fd, buffer, sizeof(buffer) - 1, 0); //leggo dati da client socket, li metto in buffer
	if (n <= 0) {
		handleClientDisconnection(index); // se n == 0 -> client ha chiuso connessione, se n < 0 -> errore (es. client disconnesso improvvisamente)
		return (false);
	}
	std::cout << "Received from client fd " << _pollFds[index].fd << ": " << std::string(buffer, n) << std::endl;
	_clients[_pollFds[index].fd].appendToBuffer(std::string(buffer, n)); // aggiungo dati al buffer del client
	std::string message;
	while (!(message = _clients[_pollFds[index].fd].extractMessageFromBuffer()).empty()) {
		std::cout << "Complete message: " << message << std::endl;
		Message msg(message); // creo oggetto Message parsando il messaggio completo
		std::cout << "Parsed message - Prefix: " << msg.getPrefix() << ", Command: " << msg.getCommand() << ", Params: ";
		for (size_t i = 0; i < msg.getParams().size(); i++) {
			std::cout << "[" << msg.getParams()[i] << "]";
		}
		// std::cout << ", Trailing: " << msg.getTrailing();
		std::cout << std::endl;
		// qui va la logica per processare il messaggio completo, es. parsing comando, esecuzione comando, invio risposta
	}
	return (true);
}

/* ------------------------------------ Utils ----------------------------------- */

void Server::addPollFd(int fd) {
	struct pollfd newPollFd;
	newPollFd.fd = fd;
	newPollFd.events = POLLIN;
	newPollFd.revents = 0;
	_pollFds.push_back(newPollFd);
}

/* ------------------------------------ Static methods ----------------------------------- */

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

// no trim spazi, IRC accetta qualsiasi stringa dopo "PASS " come password
// se un messaggio supera i 512 caratteri viene troncato, ignorando il resto
// IRC non impone criteri di sicurezza, si possono potenzialmente imporre criteri aggiuntivi
//   ma questi vanno gestiti lato server, non dipende dal protocollo IRC
bool	Server::isValidPassword(const std::string &password) {
	if (password.empty())
		return (false);
	if (password.size() > 510) // max 510 caratteri escluso "PASS " (standard IRC)
		return (false);
	return (true);
}
