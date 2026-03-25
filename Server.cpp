/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2026/03/25 17:05:40 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Message.hpp"
#include "signal.hpp"

/*
** Inizializza il server: crea il socket, lo configura e lo mette in ascolto.
**
** Sequenza obbligatoria:
**   socket()     → crea il socket TCP/IPv4, ritorna un fd
**   fcntl()      → rende il socket non-bloccante (O_NONBLOCK)
**   setsockopt() → abilita SO_REUSEADDR: evita EADDRINUSE al riavvio
**   socaddr_in   → struttura per indirizzo IPv4 (sin_family, sin_addr, sin_port), htons() per port in network byte order
**   bind()       → associa il socket all'indirizzo locale (INADDR_ANY:port)
**   listen()     → mette il socket in ascolto; SOMAXCONN = backlog massimo di sistema
**
** In caso di errore, chiude il fd aperto e lancia un'eccezione.
*/
Server::Server(const int port, const std::string &password) : _name(SERVER_NAME), _port(port), _password(password), _channels()
{
	initActions();
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0)
		throw std::runtime_error("Error creating socket");
	fcntl(_serverFd, F_SETFL, O_NONBLOCK);
	int opt = 1;
	if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0) {
		close(_serverFd);
		throw std::runtime_error("Error setting socket options");
	}
	struct sockaddr_in server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(_port);
	if (bind(_serverFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		close(_serverFd);
		throw std::runtime_error("Error binding socket");
	}
	if (listen(_serverFd, SOMAXCONN) < 0) {
		close(_serverFd);
		throw std::runtime_error("Error listening on socket");
	}
	addPollFd(_serverFd);
	std::cout << GREEN << "Server started on port " << _port << RESET << std::endl;
}

Server::~Server() {
	std::cout << "Chiusura server in corso..." << std::endl;
	for (size_t i = 0; i < _pollFds.size(); i++)
		close(_pollFds[i].fd);
}

/* ------------------------------------ Main run loop ----------------------------------- */

/*
** Main loop del server. Gira finché non arriva SIGINT (received_signal != 0).
**
** @poll        blocca finché almeno un fd ha un evento, o scade POLL_TIMEOUT
** @EINTR       se poll() viene interrotto da un segnale, riprende il loop
** @_pollFds[0] è sempre il server socket — POLLIN su di esso significa nuova connessione
** @_pollFds[i] per i > 0 sono i client socket:
**                POLLIN           → dati in arrivo → handleClientMessage()
**                POLLHUP | POLLERR → disconnessione o errore → handleClientDisconnection()
*/
void	Server::run(){
	while (received_signal == 0)
	{
		int ret = poll(&_pollFds[0], _pollFds.size(), POLL_TIMEOUT);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			std::cerr << RED <<"Error in poll()" << RESET << std::endl;
			break;
		}
		if (_pollFds[0].revents & POLLIN)
			handleNewConnection();
		for (size_t i = 1; i < _pollFds.size(); i++) {
			if (_pollFds[i].revents & POLLIN) {
				if (!handleClientMessage(i))
					i--;
			}
			else if (_pollFds[i].revents & (POLLHUP | POLLERR)) {
				handleClientDisconnection(i);
				i--;
				//tolgo client dai canali
			}
		}
	}
}

/* ------------------------------------ Handlers ----------------------------------- */

/*
** Accetta una nuova connessione in arrivo sul server socket.
**
**   accept()  → estrae la connessione dalla coda del kernel, ritorna un nuovo fd
**               dedicato a questo client (il server socket rimane in ascolto)
**   fcntl()   → rende il client fd non-bloccante
**   inet_ntoa → ricava l'hostname (stringa IP) dall'indirizzo restituito da accept()
**
** Il nuovo client viene aggiunto sia a _fds (per poll()) che a _clients (fd → Client).
*/void Server::handleNewConnection() {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientFd = accept(_serverFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
	if (clientFd < 0) {
		std::cerr << RED << "Error accepting new connection" << RESET << std::endl;
		return ;
	}
	fcntl(clientFd, F_SETFL, O_NONBLOCK);
	addPollFd(clientFd);
	std::string hostname = inet_ntoa(clientAddr.sin_addr);
	_clients[clientFd] = Client(clientFd, hostname);
	std::cout << "[fd:" << clientFd << "] New client connected from " << hostname << std::endl;
}

void Server::handleClientDisconnection(size_t index) {
	std::cout << "[fd:" << _pollFds[index].fd << "] Client disconnected" << std::endl;
	close(_pollFds[index].fd);
	_clients.erase(_pollFds[index].fd);
	_pollFds.erase(_pollFds.begin() + index);
}

/*
** Legge i dati in arrivo dal client e processa i messaggi completi.
**
**   recv()                → legge dal client fd nel buffer temporaneo s_buffer
**                           n == 0  → client ha chiuso la connessione (EOF)
**                           n <  0  → errore o disconnessione improvvisa
**                           in entrambi i casi: handleClientDisconnection()
**
**   appendToBuffer()      → accoda i byte letti al buffer persistente del client
**                           (necessario perché TCP non garantisce messaggi interi:
**                           un singolo recv() può contenere un frammento)
**
**   extractMessageFromBuffer() → estrae un messaggio completo (\r\n) dal buffer;
**                                 ritorna stringa vuota se il messaggio non è ancora completo
**
**   dispatchAction()      → parsa il messaggio e instrada al handler corretto
**
** Ritorna false se il client si è disconnesso, true altrimenti.
*/
bool Server::handleClientMessage(size_t index) {
	char s_buffer[IRC_MSG_MAX_LEN];
	int n = recv(_pollFds[index].fd, s_buffer, sizeof(s_buffer) - 1, 0);
	if (n <= 0) {
		handleClientDisconnection(index);
		return (false);
	}
	_clients[_pollFds[index].fd].appendToBuffer(std::string(s_buffer, n));
	std::string message;
	while (!(message = _clients[_pollFds[index].fd].extractMessageFromBuffer()).empty()) {
		// std::cout << "Complete message: " << message << std::endl;
		Message msg(message);
		std::cout << "[fd:" << _pollFds[index].fd << "] " << msg.getCommand() << std::endl;
		// std::cout << "Parsed message - Prefix: " << msg.getPrefix() << ", Command: " << msg.getCommand() << ", Params: ";
		// for (size_t i = 0; i < msg.getParams().size(); i++) {
		// 	std::cout << "[" << msg.getParams()[i] << "]";
		// }
		// std::cout << ", Trailing: " << msg.getTrailing();
		// std::cout << std::endl;
		dispatchAction(msg, _clients[_pollFds[index].fd]); // dispatch del messaggio al dispatcher, che processa il comando e invia eventuali risposte
	}
	return (true);
}

/* ------------------------------------ Dispatcher ----------------------------------- */

void Server::initActions()
{
	_actions["PASS"] = &Server::handlePass;
	_actions["NICK"] = &Server::handleNick;
	_actions["USER"] = &Server::handleUser;
	_actions["PING"] = &Server::handlePing;
	_actions["JOIN"] = &Server::handleJoin;
	// AGGIORNARE MAN MANO
}

// 421 ERR_UNKNOWNCOMMAND "<command> :Unknown command"
void Server::dispatchAction(const Message &msg, Client &client)
{
	if (!msg.isValid())
		return ;
	std::string command = msg.getCommand();
	std::map<std::string, void (Server::*)(const Message&, Client&)>::iterator it;
	it = _actions.find(command);
	if (it != _actions.end())
		(this->*it->second)(msg, client);
	else
	{
		sendMessageToClient(client.getFd(), "421 " + command + " :Unknown command"); //si potrebbe fare un error mananger (enum con codici di errore e messaggi)
		std::cout << "[fd:" << client.getFd() << "] " << command << " → 421" << std::endl;
	}
}

/* ------------------------------------ Commands ----------------------------------- */

// 462 ERR_ALREADYREGISTRED ":You may not reregister"
// 461 ERR_NEEDMOREPARAMS "<command> :Not enough parameters"
// 464 ERR_PASSWDMISMATCH ":Password incorrect"

void Server::handlePass(const Message &msg, Client &client) {
	if (client.getRegistered()) {
		sendMessageToClient(client.getFd(), "462 :You may not reregister");
		std::cout << "[fd:" << client.getFd() << "] PASS → 462" << std::endl;
		return ;
	}
	if (msg.getParams().size() != 1) {
		sendMessageToClient(client.getFd(), "461 " + msg.getCommand() + " :Not enough parameters");
		std::cout << "[fd:" << client.getFd() << "] PASS → 461" << std::endl;
		return ;
	}
	if (msg.getParams()[0] != _password) {
		sendMessageToClient(client.getFd(), "464 :Password incorrect");
		std::cout << "[fd:" << client.getFd() << "] PASS → 464" << std::endl;
		// handleClientDisconnection(pollfdIndexByFd(client.getFd())); //forse no? controllare protocollo
		return ;
	}
	client.setPasswordAccepted(true);
}

void Server::handleNick(const Message &msg, Client &client) {
	if (!client.getPasswordAccepted()) {
		sendMessageToClient(client.getFd(), "451 :You have not registered");
		std::cout << "[fd:" << client.getFd() << "] NICK → 451" << std::endl;
		return ;
	}
	if (msg.getParams().size() != 1) {
		sendMessageToClient(client.getFd(), "461 " + msg.getCommand() + " :Not enough parameters");
		std::cout << "[fd:" << client.getFd() << "] NICK → 461" << std::endl;
		return ;
	}
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second.getNickname() == msg.getParams()[0]) {
			sendMessageToClient(client.getFd(), "433 " + msg.getParams()[0] + " :Nickname is already in use");
			std::cout << "[fd:" << client.getFd() << "] NICK → 433" << std::endl;
			return ;
		}
	}
	if (client.getNickname().empty()) {
		client.setNickname(msg.getParams()[0]);
	}
	else if (client.getRegistered()) {
		std::string oldNickname = client.getNickname();
		client.setNickname(msg.getParams()[0]);
		std::string nicknameChangeMsg = ":" + oldNickname + " NICK :" + client.getNickname();
		sendMessageToClient(client.getFd(), nicknameChangeMsg);
	}
	if (!client.getRegistered() && !client.getUsername().empty() && !client.getNickname().empty() && !client.getHostname().empty()) {
		client.setRegistered(true);
		sendWelcomeMessage(client);
	}
}

void Server::handleUser(const Message &msg, Client &client) {
	if (!client.getPasswordAccepted()) {
		sendMessageToClient(client.getFd(), "451 :You have not registered");
		std::cout << "[fd:" << client.getFd() << "] USER → 451" << std::endl;
		return ;
	}
	if (msg.getParams().size() < 3) {
		sendMessageToClient(client.getFd(), "461 " + msg.getCommand() + " :Not enough parameters");
		std::cout << "[fd:" << client.getFd() << "] USER → 461" << std::endl;
		return ;
	}
	if (client.getRegistered()) {
		sendMessageToClient(client.getFd(), "462 :You may not reregister");
		std::cout << "[fd:" << client.getFd() << "] USER → 462" << std::endl;
		return ;
	}
	client.setUsername(msg.getParams()[0]);
	client.setHostname(msg.getParams()[1]);
	client.setRealname(msg.getTrailing());
	if (!client.getNickname().empty()) {
		client.setRegistered(true);
		sendWelcomeMessage(client);
	}
}

// es. client manda "PONG :123"
//     server risponde "PONG :123"
// no params -> ERR_NOORIGIN
// max 2 params -> se più di 2, ignora quelli extra
void Server::handlePing(const Message &msg, Client &client)
{
	if (msg.getParams().empty()) {
		sendMessageToClient(client.getFd(), "409 :No origin specified"); // ERR_NOORIGIN
		std::cout << "[fd:" << client.getFd() << "] PING → 409" << std::endl;
		return;
	}
	std::string message = "PONG " + msg.getParams()[0];
	if (msg.getParams().size() > 1)
		message += " " + msg.getParams()[1];
	sendMessageToClient(client.getFd(), message);
}


void Server::handleJoin(const Message &msg, Client &client)
{
	// autenticato
	if (!client.getPasswordAccepted()) {
		sendMessageToClient(client.getFd(), "451 :You have not registered");
		return ;
	}

	(void)msg;
	// no params

	// canale non esiste

	// canale esiste -> Channel.handleJoin(client)

	// canale non esiste -> Channel.create(client)

}

/* ------------------------------------ Channel ----------------------------------- */

Channel* Server::getChannelByName(const std::string &name)
{
	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (_channels[i].getName() == name)
			return &_channels[i];
	}
	return NULL;
}

void Server::printChannels()
{
	for (size_t i = 0; i < _channels.size(); i++)
		_channels[i].printChannelInfo();
}

/* ------------------------------------ Utils ----------------------------------- */

void Server::addPollFd(int fd) {
	struct pollfd newPollFd;
	newPollFd.fd = fd;
	newPollFd.events = POLLIN;
	newPollFd.revents = 0;
	_pollFds.push_back(newPollFd);
}

void Server::sendMessageToClient(int fd, const std::string &message) {
	std::string msgWithCRLF = message + "\r\n";
	send(fd, msgWithCRLF.c_str(), msgWithCRLF.size(), 0);
}

size_t Server::pollfdIndexByFd(int fd) {
	for (size_t i = 0; i < _pollFds.size(); i++) {
		if (_pollFds[i].fd == fd)
			return (i);
	}
	return (_pollFds.size()); // ritorna un indice fuori range se non trovato
}

void Server::sendWelcomeMessage(const Client &client) {
	std::string nick = client.getNickname();
	std::string host = "ircserv";

	sendMessageToClient(client.getFd(), ":" + host + " 001 " + nick + " :Welcome to the IRC network, " + nick + "!");
	sendMessageToClient(client.getFd(), ":" + host + " 002 " + nick + " :Your host is " + host + ", running version 1.0");
	sendMessageToClient(client.getFd(), ":" + host + " 003 " + nick + " :This server was created " + std::string(__DATE__));
	sendMessageToClient(client.getFd(), ":" + host + " 004 " + nick + " " + host + " 1.0 o o");
}

int Server::getFdByNickname(const std::string &nickname)
{
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.getNickname() == nickname)
            return it->first;
    }
    return -1;
}

/* ------------------------------------ Static methods ----------------------------------- */

/*
** Valida la porta passata come argomento da riga di comando.
**
** Le porte sono numeri a 16 bit (0–65535, ovvero 2¹⁶ - 1).
** Le porte 0–1023 sono riservate al sistema operativo (well-known ports)
** e richiedono privilegi di root — accettiamo solo porte > 1024.
**
*/
bool	Server::isValidPort(const std::string &port) {
	if (port.empty() || port.size() > 5)
		return (false);
	for (size_t i = 0; i < port.size(); i++) {
		if (!std::isdigit(port[i]))
			return (false);
	}
	int port_num = std::atoi(port.c_str());
	return (port_num > 1024 && port_num <= 65535);
}

/*
** Valida la password passata come argomento da riga di comando.
**
** IRC non impone criteri di sicurezza sulla password — qualsiasi stringa
** non vuota è tecnicamente valida dopo "PASS ".
** Non viene fatto trim degli spazi: la password viene confrontata così com'è.
**
** Il limite di 510 caratteri deriva dallo standard IRC: i messaggi hanno
** lunghezza massima di 512 byte incluso \r\n, quindi "PASS " occupa 5 byte
** e ne restano 510 per la password stessa. Messaggi più lunghi verrebbero
** troncati dal protocollo, rendendo l'autenticazione impossibile.
*/
bool	Server::isValidPassword(const std::string &password) {
	if (password.empty())
		return (false);
	if (password.size() > 510)
		return (false);
	return (true);
}

// int	Server::join(const Message &msg, const Client &client)
// {
// 	size_t		i = 0;
// 	if (msg.getParams().size() > 1)
// 		Server::sendMessageToClient(client.getFd(), "461 " + client.getNickname() + " " + msg.getCommand() + " :Not enough parameters"); //ERR_BADCHANMASK se non vogliamo gestire gli spazzi
// 	std::string channels = msg.getParams()[0];
// 	while (!channels.empty()) {
// 		if (channels[0] != '#') {
// 			Server::sendMessageToClient(client.getFd(), "403 " + client.getNickname() + " " + msg.getCommand() + " :No such channel"); //ERR_NOSUCHCHANNEL deve iniziare con # channel
// 		}
// 		size_t pos = channels.find(',');
// 		std::string channel = channels.substr(0, pos); //estraggo il channel
// 		/*if (!channe.exist(channel))
// 			Server::sendMessageToClient(client.getFd(), "403 " + client.getNickname() + " " + msg.getCommand() + " :No such channel");
// 		*/
// 		channels = channels.substr(pos + 1);
// 	}
// }
