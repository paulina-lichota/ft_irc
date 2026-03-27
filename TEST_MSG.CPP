/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TEST_MSG.CPP                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: francema <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 21:25:20 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/27 10:00:20 by francema         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "includes.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include "Channel.hpp"

class Server {
	private:
		enum LogType {
			LOG_IN,
			LOG_OUT,
			LOG_SERVER,
			LOG_DEBUG
		};
		std::string					_name; // nome del server, usato nei prefix dei messaggi (es. :ircserv 001 nickname :Welcome to the IRC server)
		int							_port;
		std::string					_password; // PASS mypassword -> mypassword == _passoword, altrimenti connessione rifiutata
		int							_serverFd; // socket del server (ascolta nuovi client)
		std::vector<struct pollfd>	_pollFds; // array di fd dei clients da monitorare
		std::map<int, Client>		_clients;	 // associazione fd -> oggetto client
		static const int			POLL_TIMEOUT = -1; // -1 per aspettare eventi all'infinito
		std::vector<Channel>		_channels; // MAP CON NAME? PER MIGLIORARE COMPLESSITÀ RICERCA  lista di canali esistenti, ogni canale ha una lista di client connessi
		std::map<std::string, void (Server::*)(const Message&, Client&)> _actions; // mappa comando + pointer a funzione
	public:
		Server(const int port, const std::string &password);
		~Server();

		void run();

		//LOGS
		void logIn(const Client& c, const std::string& msg);
		void logOut(const Client& c, const std::string& msg);
		void log(LogType type, const Client* client, const std::string& msg);
		void errlog(Client &client, const std::string &code, const std::string &target, const std::string &message);

		// Handlers:
		void handleNewConnection();
		void handleClientDisconnection(size_t index);
		bool handleClientMessage(size_t index); // ritorna false se client si è disconnesso, true altrimenti

		// Utility:
		void addPollFd(int fd);
		void sendMessageToClient(int fd, const std::string &message);
		size_t pollfdIndexByFd(int fd);
		void sendWelcomeMessage(const Client &client);
		int getFdByNickname(const std::string &nickname);

		// Static methods:
		static bool isValidPort(const std::string &port);
		static bool isValidPassword(const std::string &password);
		static bool isValidNickname(const std::string &nickname);
		static bool isValidChannelName(const std::string &channelName);

		// Channels
		void printChannels();
		Channel* getChannelByName(const std::string &name); // ritorna NULL se non esiste
		void createChannel(const std::string &name);

		// invia message a tutti i client del canale tranne toExclude. sender già dentro la stringa
		void broadcastMessageToChannel(const std::string &message, const Channel &channel, const std::string &toExclude);

		/*
		All the messages sent from one client to a channel have to be forwarded to every other client that joined the channel.
		*/
		void applyMode(const Message &msg, Channel &channel, Client &client);

		// Dispatcher
		void initActions();
		void dispatchAction(const Message &msg, Client &client);

		// Actions
		void handlePass(const Message &msg, Client &client);
		void handleNick(const Message &msg, Client &client);
		void handleUser(const Message &msg, Client &client);
		void handlePing(const Message &msg, Client &client);
		void handleQuit(const Message &msg, Client &client);
		void handleJoin(const Message &msg, Client &client);
		void handlePrivmsg(const Message &msg, Client &client);
		void handleTopic(const Message &msg, Client &client);
		void handleMode(const Message &msg, Client &client);
		// Operator actions:
		void handleKick(const Message &msg, Client &client);
		void handleInvite(const Message &msg, Client &client); // invita un client al can

};

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: francema <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2026/03/27 09:56:35 by francema         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
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
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		sendMessageToClient(it->first, "ERROR :Server shutting down");
	}
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

void Server::logIn(const Client& c, const std::string& msg) {
	log(LOG_IN, &c, msg);
}

void Server::logOut(const Client& c, const std::string& msg) {
	log(LOG_OUT, &c, msg);
}

void Server::log(LogType type, const Client* client, const std::string& msg) {
	std::string prefix;

	// Tipo log
	switch (type) {
		case LOG_IN:     prefix = "[IN]"; break;
		case LOG_OUT:    prefix = "[OUT]"; break;
		case LOG_SERVER: prefix = "[SERVER]"; break;
		case LOG_DEBUG:  prefix = "[DEBUG]"; break;
	}
	// Info client
	std::string clientInfo = "[fd:?]";
	std::string identity;
	if (client) {
		clientInfo = "[fd:" + std::to_string(client->getFd()) + "]";
		if (!client->getNickname().empty())
			identity = "[" + client->getPrefix() + "]";
	}
	std::cout << clientInfo << prefix << identity << " " << msg << std::endl;
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
*/
void Server::handleNewConnection() {
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientFd = accept(_serverFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
	if (clientFd < 0) {
		log(LOG_SERVER, NULL, "Error accepting new connection");
		return ;
	}
	fcntl(clientFd, F_SETFL, O_NONBLOCK);
	addPollFd(clientFd);
	std::string hostname = inet_ntoa(clientAddr.sin_addr);
	_clients[clientFd] = Client(clientFd, hostname);
	log(LOG_SERVER, &_clients[clientFd], "New client connected from " + hostname);
}

void Server::handleClientDisconnection(size_t index) {
	int fd = _pollFds[index].fd;
	std::string nick = _clients[fd].getNickname();

	// rimuovi il client da tutti i canali
	for (size_t i = 0; i < _channels.size(); i++) {
		_channels[i].removeMember(nick);
		_channels[i].removeOperator(nick);
		_channels[i].removeInvited(nick);
	}
	// elimina i canali rimasti vuoti
	for (size_t i = 0; i < _channels.size();) {
		if (_channels[i].getMemberCount() == 0)
			_channels.erase(_channels.begin() + i);
		else
			i++;
	}

	log(LOG_SERVER, &_clients[fd], "Client disconnected");
	//close(fd);?
	_clients.erase(fd);
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
	int fd = _pollFds[index].fd;  // salva fd subito
	char s_buffer[IRC_MSG_MAX_LEN];
	int n = recv(fd, s_buffer, sizeof(s_buffer) - 1, 0);
	if (n <= 0) {
		handleClientDisconnection(index);
		return (false);
	}
	_clients[fd].appendToBuffer(std::string(s_buffer, n));
	std::string message;
	while (!(message = _clients[fd].extractMessageFromBuffer()).empty()) {
		logIn(_clients[fd], message);
		Message msg(message);
		log(LOG_DEBUG, &_clients[fd], "CMD: " + msg.getCommand());
		dispatchAction(msg, _clients[fd]);
		if (_clients.find(fd) == _clients.end())  // QUIT ha rimosso il client
			return (false);
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
	_actions["PRIVMSG"] = &Server::handlePrivmsg;
	_actions["TOPIC"] = &Server::handleTopic;
	_actions["MODE"] = &Server::handleMode;
	_actions["KICK"] = &Server::handleKick;
	_actions["QUIT"] = &Server::handleQuit;// gestisce anche il caso in cui il client si disconnette senza inviare QUIT, ma semplicemente chiudendo la connessione
	_actions["INVITE"] = &Server::handleInvite;
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
		log(LOG_SERVER, &client, command + " → 421");
	}
}

/* ------------------------------------ Commands ----------------------------------- */

// 462 ERR_ALREADYREGISTRED ":You may not reregister"
// 461 ERR_NEEDMOREPARAMS "<command> :Not enough parameters"
// 464 ERR_PASSWDMISMATCH ":Password incorrect"

void Server::handlePass(const Message &msg, Client &client) {
	if (client.getRegistered()) {
		sendMessageToClient(client.getFd(), "462 :You may not reregister");
		log(LOG_SERVER, &client, "PASS → 462");
		return ;
	}
	if (msg.getParams().size() != 1) {
		sendMessageToClient(client.getFd(), "461 " + msg.getCommand() + " :Not enough parameters");
		log(LOG_SERVER, &client, "PASS → 461");
		return ;
	}
	if (msg.getParams()[0] != _password) {
		sendMessageToClient(client.getFd(), "464 :Password incorrect");
		log(LOG_SERVER, &client, "PASS → 464");
		// handleClientDisconnection(pollfdIndexByFd(client.getFd()));
		return ;
	}
	client.setPasswordAccepted(true);
	log(LOG_SERVER, &client, "PASS → OK");
}

void Server::handleNick(const Message &msg, Client &client) {
	if (!client.getPasswordAccepted()) {
		sendMessageToClient(client.getFd(), ":" + _name + " 451 " + client.getNickname() + " :You have not registered");
		log(LOG_SERVER, &client, "NICK → 451");
		return ;
	}
	if (msg.getParams().size() != 1) {
		sendMessageToClient(client.getFd(), "461 " + msg.getCommand() + " :Not enough parameters");
		log(LOG_SERVER, &client, "NICK → 461");
		return ;
	}
	if (!isValidNickname(msg.getParams()[0])) {
		sendMessageToClient(client.getFd(), "432 " + msg.getParams()[0] + " :Erroneous nickname");
		log(LOG_SERVER, &client, "NICK → 432");
		return;
	}
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second.getNickname() == msg.getParams()[0]) {
			sendMessageToClient(client.getFd(), "433 " + msg.getParams()[0] + " :Nickname is already in use");
			log(LOG_SERVER, &client, "NICK → 433");
			return ;
		}
	}

	if (!client.getRegistered()) {
		client.setNickname(msg.getParams()[0]);
		log(LOG_SERVER, &client, "NICK set (pre-registration)");
	} else {
		std::string oldNickname = client.getNickname();
		client.setNickname(msg.getParams()[0]);

		std::string nicknameChangeMsg = ":" + oldNickname + "!" + client.getUsername() + "@" + client.getHostname() + " NICK :" + client.getNickname();

		sendMessageToClient(client.getFd(), nicknameChangeMsg);
		log(LOG_SERVER, &client, "NICK changed from " + oldNickname + " to " + client.getNickname());

		for (size_t i = 0; i < _channels.size(); i++) {
			Channel &ch = _channels[i];
			bool wasMember = ch.isMember(oldNickname);
			bool wasOperator = ch.isOperator(oldNickname);
			bool wasInvited = ch.isInvited(oldNickname);

			if (wasMember) {
				broadcastMessageToChannel(nicknameChangeMsg, ch, oldNickname);
				ch.removeMember(oldNickname);
				ch.addMember(client.getNickname());
			}
			if (wasOperator) {
				ch.removeOperator(oldNickname);
				ch.addOperator(client.getNickname());
			}
			if (wasInvited) {
				ch.removeInvited(oldNickname);
				ch.addInvited(client.getNickname());
			}
		}
	}

	if (!client.getRegistered() && !client.getUsername().empty() && !client.getNickname().empty() && !client.getHostname().empty())
	{
		client.setRegistered(true);
		log(LOG_SERVER, &client, "Client registered");
		sendWelcomeMessage(client);
	}
}

void Server::handleUser(const Message &msg, Client &client) {
	if (!client.getPasswordAccepted()) {
		sendMessageToClient(client.getFd(), "451 :You have not registered");
		log(LOG_SERVER, &client, "USER → 451");
		return ;
	}
	if (msg.getParams().size() < 3) {
		sendMessageToClient(client.getFd(), "461 " + msg.getCommand() + " :Not enough parameters");
		log(LOG_SERVER, &client, "USER → 461");
		return ;
	}
	if (client.getRegistered()) {
		sendMessageToClient(client.getFd(), "462 :You may not reregister");
		log(LOG_SERVER, &client, "USER → 462");
		return ;
	}

	client.setRealname(msg.getTrailing());
	log(LOG_SERVER, &client, "USER set realname");

	if (!client.getNickname().empty()) {
		client.setRegistered(true);
		log(LOG_SERVER, &client, "Client registered");
		sendWelcomeMessage(client);
	}
}

// es. client manda "PONG :123"
//     server risponde "PONG :123"
// no params -> ERR_NOORIGIN
// max 2 params -> se più di 2, ignora quelli extra
void Server::handlePing(const Message &msg, Client &client) {
	std::string token;

	if (!msg.getParams().empty())
		token = msg.getParams()[0];
	else if (msg.hasTrailing())
		token = msg.getTrailing();
	else {
		sendMessageToClient(client.getFd(), ":" + _name + " 409 " + client.getNickname() + " :No origin specified");
		log(LOG_SERVER, &client, "PING → 409 (no origin)");
		return;
	}

	sendMessageToClient(client.getFd(), ":" + _name + " PONG " + _name + " :" + token);
	log(LOG_SERVER, &client, "PING → PONG (" + token + ")");
}

/*
** Gestisce il comando QUIT: il client vuole disconnettersi volontariamente.
**
** La disconnessione "pulita" tramite QUIT è diversa da una disconnessione
** brusca (recv() == 0 o POLLHUP): in questo caso il client manda esplicitamente
** il comando, quindi il server deve prima notificare tutti i canali coinvolti
** e poi chiudere la connessione.
**
** Sequenza:
**   1. Costruisce il messaggio QUIT con il prefix completo nick!user@host
**      e il quit message (trailing del comando, oppure "Client quit" di default)
**   2. Broadcast del messaggio a tutti i canali di cui il client è membro,
**      escludendo il client stesso (che riceverà il suo messaggio al passo 3)
**   3. Invia il messaggio QUIT anche al client che ha mandato il comando
**      (HexChat si aspetta di ricevere il proprio QUIT prima che la connessione chiuda)
**   4. Chiama handleClientDisconnection() che fa il cleanup: rimuove il client
**      dai canali, chiude il fd, rimuove da _clients e _pollFds
*/
void Server::handleQuit(const Message &msg, Client &client) {
	std::string quitMsg = "Client quit";
	if (msg.hasTrailing()) {
		quitMsg = msg.getTrailing();
		if (quitMsg.length() > 509)
			quitMsg = quitMsg.substr(0, 509);
	}
	std::string quitMessage = client.getPrefix() + " QUIT :" + quitMsg;
	for (size_t i = 0; i < _channels.size(); i++) {
		if (_channels[i].isMember(client.getNickname()))
			broadcastMessageToChannel(quitMessage, _channels[i], client.getNickname());
	}
	sendMessageToClient(client.getFd(), quitMessage);
	log(LOG_SERVER, &client, "QUIT (" + quitMsg + ")");
	size_t idx = pollfdIndexByFd(client.getFd());
	if (idx < _pollFds.size())
		handleClientDisconnection(idx);
}

// es. client manda "JOIN #channel"
//     server risponde "JOIN #channel"
// può mandare anche "JOIN #channel password" se il canale è protetto da password
// no params -> ERR_NEEDMOREPARAMS
// canale non esiste Channel.create(client) → Channel.handleJoin(client)
// canale esiste -> Channel.handleJoin(client)
void Server::handleJoin(const Message &msg, Client &client)
{
	// autenticato
	if (!client.getRegistered()) {
		sendMessageToClient(client.getFd(), "451 :You have not registered");
		log(LOG_SERVER, &client, "JOIN → 451");
		return ;
	}

	// no channel name
	if (msg.getParams().size() == 0) {
		sendMessageToClient(client.getFd(), "461 " + msg.getCommand() + " :Not enough parameters");
		log(LOG_SERVER, &client, "JOIN → 461");
		return ;
	}

	std::string channelName = msg.getParams()[0];
	Channel *channel = getChannelByName(channelName);

	// canale non esiste
	if (channel == NULL) {
		if (!isValidChannelName(channelName)) {
			sendMessageToClient(client.getFd(), "403 " + msg.getCommand() + " :No such channel");
			log(LOG_SERVER, &client, "JOIN → 403 (invalid channel)");
			return ;
		}
		createChannel(channelName);
		channel = getChannelByName(channelName);
		log(LOG_SERVER, &client, "JOIN created channel " + channelName);
	}

	// key
	if (!channel->getKey().empty())
	{
		std::string key = "";
		if (msg.getParams().size() > 1)
			key = msg.getParams()[1];

		if (key != channel->getKey()) {
			sendMessageToClient(client.getFd(), ":" + _name + " 475 " + client.getNickname() + " " + channelName + " :Cannot join channel (+k)");
			log(LOG_SERVER, &client, "JOIN → 475 (bad key)");
			return;
		}
	}

	// già membro
	if (channel->isMember(client.getNickname())) {
		log(LOG_SERVER, &client, "JOIN ignored (already member)");
		return;
	}

	// invite only
	if (channel->getInviteOnly() && !channel->isInvited(client.getNickname())) {
		sendMessageToClient(client.getFd(), "473 " + client.getNickname() + " " + channelName + " :Cannot join channel (+i)");
		log(LOG_SERVER, &client, "JOIN → 473 (invite only)");
		return;
	}

	// limit
	if (channel->getUsersLimit() > 0 && channel->getMemberCount() >= channel->getUsersLimit()) {
		sendMessageToClient(client.getFd(), "471 " + client.getNickname() + " " + channelName + " :Cannot join channel (+l)");
		log(LOG_SERVER, &client, "JOIN → 471 (channel full)");
		return;
	}

	// aggiunta membro
	channel->addMember(client.getNickname());

	if (channel->getMemberCount() == 1)
		channel->addOperator(client.getNickname());

	if (channel->isInvited(client.getNickname()))
		channel->removeInvited(client.getNickname());

	log(LOG_SERVER, &client, "JOIN success " + channelName);

	// broadcast
	const std::string message = client.getPrefix() + " JOIN " + channelName;
	broadcastMessageToChannel(message, *channel, "");

	// names list
	std::string namesList = "";
	const std::set<std::string> &members = channel->getMembers();

	for (std::set<std::string>::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		if (channel->isOperator(*it))
			namesList += "@";
		namesList += *it + " ";
	}

	sendMessageToClient(client.getFd(), ":" + _name + " 353 " + client.getNickname() + " = " + channelName + " :" + namesList);
	sendMessageToClient(client.getFd(), ":" + _name + " 366 " + client.getNickname() + " " + channelName + " :End of /NAMES list");
}


void Server::handlePrivmsg(const Message &msg, Client &client)
{
	if (msg.getParams().empty()) {
		sendMessageToClient(client.getFd(), ":" + _name + " 461 " + client.getNickname() + " PRIVMSG :Not enough parameters");
		log(LOG_SERVER, &client, "PRIVMSG → 461");
		return ;
	}

	std::string target = msg.getParams()[0];

	// ===================== CHANNEL =====================
	if (!target.empty() && (target[0] == '#' || target[0] == '&'))
	{
		Channel *channel = getChannelByName(target);

		if (!channel) {
			sendMessageToClient(client.getFd(),
				":" + _name + " 403 " + client.getNickname() + " " + target + " :No such channel");
			log(LOG_SERVER, &client, "PRIVMSG → 403 (no such channel " + target + ")");
			return;
		}

		if (!channel->isMember(client.getNickname())) {
			sendMessageToClient(client.getFd(),
				":" + _name + " 404 " + client.getNickname() + " " + target + " :Cannot send to channel");
			log(LOG_SERVER, &client, "PRIVMSG → 404 (not in channel " + target + ")");
			return;
		}

		if (msg.getTrailing().empty()) {
			sendMessageToClient(client.getFd(),
				":" + _name + " 412 " + client.getNickname() + " :No text to send");
			log(LOG_SERVER, &client, "PRIVMSG → 412 (empty message)");
			return ;
		}

		std::string message = client.getPrefix() + " PRIVMSG " + target + " :" + msg.getTrailing();
		broadcastMessageToChannel(message, *channel, client.getNickname());

		log(LOG_SERVER, &client, "PRIVMSG to channel " + target);
	}

	// ===================== NICK =====================
	else {
		int targetFd = getFdByNickname(target);

		if (targetFd == -1) {
			sendMessageToClient(client.getFd(),
				":" + _name + " 401 " + client.getNickname() + " " + target + " :No such nick/channel");
			log(LOG_SERVER, &client, "PRIVMSG → 401 (no such nick " + target + ")");
			return;
		}

		if (msg.getTrailing().empty()) {
			sendMessageToClient(client.getFd(),
				":" + _name + " 412 " + client.getNickname() + " :No text to send");
			log(LOG_SERVER, &client, "PRIVMSG → 412 (empty message)");
			return ;
		}

		std::string message = client.getPrefix() + " PRIVMSG " + target + " :" + msg.getTrailing();
		sendMessageToClient(targetFd, message);

		log(LOG_SERVER, &client, "PRIVMSG to user " + target);
	}
}

void Server::handleTopic(const Message &msg, Client &client)
{
	// log(LOG_DEBUG, &client,
	// 	"TOPIC params=" + std::to_string(msg.getParams().size()) +
	// 	" trailing=" + (msg.hasTrailing() ? msg.getTrailing() : "NULL"));
	// es. TOPIC
	if (msg.getParams().empty()) {
		sendMessageToClient(client.getFd(),
			":" + _name + " 461 " + msg.getCommand() + " :Not enough parameters");
		log(LOG_SERVER, &client, "TOPIC → 461");
		return ;
	}
	std::string channelName = msg.getParams()[0];
	Channel *channel = getChannelByName(channelName);
	// channel non esiste
	if (channel == NULL) {
		sendMessageToClient(client.getFd(),
			":" + _name + " 403 " + msg.getCommand() + " :No such channel");
		log(LOG_SERVER, &client, "TOPIC → 403 (no such channel " + channelName + ")");
		return ;
	}
	// SOLO LETTURA TOPIC
	if (msg.getParams().size() == 1 && !msg.hasTrailing())
	{
		if (channel->getTopic().empty()) {
			sendMessageToClient(client.getFd(),
				":" + _name + " 331 " + client.getNickname() + " " + channelName + " :No topic is set");
			log(LOG_SERVER, &client, "TOPIC read → 331 (no topic)");
		}
		else {
			sendMessageToClient(client.getFd(),
				":" + _name + " 332 " + client.getNickname() + " " + channelName + " :" + channel->getTopic());
			log(LOG_SERVER, &client, "TOPIC read → 332");
		}
		return;
	}
	// non membro
	if (!channel->isMember(client.getNickname())) {
		sendMessageToClient(client.getFd(),
			"442 " + channelName + " :You're not on that channel");
		log(LOG_SERVER, &client, "TOPIC → 442 (not in channel)");
		return ;
	}
	// no permessi
	if (channel->getTopicRestricted() && !channel->isOperator(client.getNickname())) {
		sendMessageToClient(client.getFd(),
			"482 " + channelName + " :You're not channel operator");
		log(LOG_SERVER, &client, "TOPIC → 482 (not operator)");
		return ;
	}
	// set topic
	channel->setTopic(msg.getTrailing());
	std::string topicMsg = client.getPrefix() + " TOPIC " + channelName + " :" + msg.getTrailing();
	broadcastMessageToChannel(topicMsg, *channel, "");
	log(LOG_SERVER, &client, "TOPIC set on " + channelName);
}

void Server::handleMode(const Message &msg, Client &client)
{
	// log(LOG_DEBUG, &client,
	// 	"MODE params=" + std::to_string(msg.getParams().size()) +
	// 	" trailing=" + (msg.hasTrailing() ? msg.getTrailing() : "NULL"));
	// es. MODE
	if (msg.getParams().empty()) {
		sendMessageToClient(client.getFd(),
			":" + _name + " 461 " + msg.getCommand() + " :Not enough parameters");
		log(LOG_SERVER, &client, "MODE → 461");
		return ;
	}
	std::string channelName = msg.getParams()[0];
	Channel *channel = getChannelByName(channelName);
	// channel non esiste
	if (channel == NULL) {
		sendMessageToClient(client.getFd(),
			":" + _name + " 403 " + msg.getCommand() + " :No such channel");
		log(LOG_SERVER, &client, "MODE → 403 (no such channel " + channelName + ")");
		return ;
	}
	// non membro
	if (!channel->isMember(client.getNickname()))
	{
		sendMessageToClient(client.getFd(),
			"442 " + channelName + " :You're not on that channel");
		log(LOG_SERVER, &client, "MODE → 442 (not in channel)");
		return ;
	}
	// solo lettura modes
	if (msg.getParams().size() == 1)
	{
		std::string modes = channel->getModes();

		sendMessageToClient(client.getFd(),
			":" + _name + " 324 " + client.getNickname() + " " + channelName + " " + modes);

		log(LOG_SERVER, &client, "MODE read " + channelName + " = " + modes);
		return ;
	}
	// non operatore
	if (!channel->isOperator(client.getNickname()))
	{
		sendMessageToClient(client.getFd(),
			"482 " + channelName + " :You're not channel operator");
		log(LOG_SERVER, &client, "MODE → 482 (not operator)");
		return ;
	}
	// SET MODE
	log(LOG_SERVER, &client, "MODE set on " + channelName +
		" flags=" + msg.getParams()[1]);
	applyMode(msg, *channel, client);
}

/* ------------------------------------ Modes ----------------------------------- */

static void addToAppliedModes(std::string &appliedModes, char &lastSign, bool adding, char mode)
{
	char sign = adding ? '+' : '-';
	if (lastSign != sign) // aggiungo un segno solo se cambio
	{
		appliedModes += sign;
		lastSign = sign;
	}
	appliedModes += mode;
}

// check operator gia' effettuato prima di chiamare questa funzione
void Server::applyMode(const Message &msg, Channel &channel, Client &client)
{
	std::string appliedModes = "";
	std::string appliedParams = "";
	char lastSign = 0;

	std::string mode = msg.getParams()[1];
	size_t paramIndex = 2; // indice del primo parametro dopo mode

	// log dei parametri ricevuti
	log(LOG_DEBUG, &client, "applyMode: mode=" + mode + ", total_params=" + std::to_string(msg.getParams().size()));
	for (size_t i = 0; i < msg.getParams().size(); i++) {
		log(LOG_DEBUG, &client, "param[" + std::to_string(i) + "]=" + msg.getParams()[i]);
	}

	bool adding = true; // se manca, + di default
	for (size_t i = 0; i < mode.size(); i++)
	{
		if (mode[i] == '+') { adding = true; continue; }
		if (mode[i] == '-') { adding = false; continue; }

		if (mode[i] == 'i') {
			channel.setInviteOnly(adding);
			addToAppliedModes(appliedModes, lastSign, adding, 'i');
			log(LOG_SERVER, &client, "MODE i → " + std::string(adding ? "set" : "unset"));
		}
		else if (mode[i] == 't') {
			channel.setTopicRestricted(adding);
			addToAppliedModes(appliedModes, lastSign, adding, 't');
			log(LOG_SERVER, &client, "MODE t → " + std::string(adding ? "set" : "unset"));
		}
		else if (mode[i] == 'k') {
			if (adding) {
				if (paramIndex >= msg.getParams().size()) {
					sendMessageToClient(client.getFd(), ":" + _name + " 461 " + client.getNickname() + " MODE :Not enough parameters");
					log(LOG_SERVER, &client, "MODE k → missing key parameter");
					continue;
				}
				std::string key = msg.getParams()[paramIndex++];
				channel.setKey(key);
				addToAppliedModes(appliedModes, lastSign, adding, 'k');
				appliedParams += " " + key;
				log(LOG_SERVER, &client, "MODE k → set key=" + key);
			} else {
				channel.setKey("");
				addToAppliedModes(appliedModes, lastSign, adding, 'k');
				log(LOG_SERVER, &client, "MODE k → unset key");
			}
		}
		else if (mode[i] == 'l') {
			if (adding) {
				if (paramIndex >= msg.getParams().size()) {
					sendMessageToClient(client.getFd(), ":" + _name + " 461 " + client.getNickname() + " MODE :Not enough parameters");
					log(LOG_SERVER, &client, "MODE l → missing limit parameter");
					continue;
				}
				std::string limitstr = msg.getParams()[paramIndex++];
				if (!channel.isValidLimit(limitstr)) {
					sendMessageToClient(client.getFd(), ":" + _name + " 461 " + client.getNickname() + " MODE :Invalid limit");
					log(LOG_SERVER, &client, "MODE l → invalid limit=" + limitstr);
					continue;
				}
				size_t limit = atoi(limitstr.c_str());
				channel.setUsersLimit(limit);
				appliedParams += " " + limitstr;
				log(LOG_SERVER, &client, "MODE l → set limit=" + limitstr);
			} else {
				channel.setUsersLimit(0);
				log(LOG_SERVER, &client, "MODE l → unset limit");
			}
			addToAppliedModes(appliedModes, lastSign, adding, 'l');
		}
		else if (mode[i] == 'o') {
			if (paramIndex >= msg.getParams().size()) {
				sendMessageToClient(client.getFd(), ":" + _name + " 461 " + client.getNickname() + " MODE :Not enough parameters");
				log(LOG_SERVER, &client, "MODE o → missing user parameter");
				continue;
			}
			std::string member = msg.getParams()[paramIndex++];
			if (!channel.isMember(member)) {
				sendMessageToClient(client.getFd(), ":" + _name + " 441 " + member + " " + channel.getName() + " :They aren't on that channel");
				log(LOG_SERVER, &client, "MODE o → user not in channel: " + member);
				continue;
			}
			if (adding) channel.addOperator(member);
			else channel.removeOperator(member);
			appliedParams += " " + member;
			addToAppliedModes(appliedModes, lastSign, adding, 'o');
			log(LOG_SERVER, &client, "MODE o → " + std::string(adding ? "add operator" : "remove operator") + " " + member);
		}
		else {
			sendMessageToClient(client.getFd(), ":" + _name + " 472 " + client.getNickname() + " :Unknown MODE flag");
			log(LOG_SERVER, &client, "MODE unknown flag: " + std::string(1, mode[i]));
			continue;
		}
	}

	broadcastMessageToChannel(":" + client.getPrefix() + " MODE " + channel.getName() + " " + appliedModes + appliedParams, channel, "");
	log(LOG_SERVER, &client, "MODE broadcasted: " + appliedModes + appliedParams);
}

/* ------------------------------------ Operator actions ----------------------------------- */

void Server::handleKick(const Message &msg, Client &client) {
	if (!client.getRegistered()) {
		sendMessageToClient(client.getFd(), "451 :You have not registered");
		log(LOG_SERVER, &client, "KICK → 451 not registered");
		return;
	}

	if (msg.getParams().size() < 2) {
		sendMessageToClient(client.getFd(), "461 " + msg.getCommand() + " :Not enough parameters");
		log(LOG_SERVER, &client, "KICK → 461 not enough parameters");
		return;
	}

	std::string channelName = msg.getParams()[0];
	Channel *channel = getChannelByName(channelName);

	if (channel == NULL) {
		sendMessageToClient(client.getFd(), "403 " + msg.getCommand() + " :No such channel");
		log(LOG_SERVER, &client, "KICK → 403 no such channel: " + channelName);
		return;
	}

	if (!channel->isOperator(client.getNickname())) {
		sendMessageToClient(client.getFd(), "482 " + channelName + " :You're not channel operator");
		log(LOG_SERVER, &client, "KICK → 482 not operator in channel: " + channelName);
		return;
	}

	std::string targetNickname = msg.getParams()[1];
	if (!channel->isMember(targetNickname)) {
		sendMessageToClient(client.getFd(), "441 " + targetNickname + " " + channelName + " :They aren't on that channel");
		log(LOG_SERVER, &client, "KICK → 441 target not in channel: " + targetNickname);
		return;
	}

	// Costruisco e invio il messaggio KICK
	std::string kickMessage = client.getPrefix() + " KICK " + channelName + " " + targetNickname;
	if (!msg.getTrailing().empty()) {
		kickMessage += " :" + msg.getTrailing();
	}
	broadcastMessageToChannel(kickMessage, *channel, "");

	// Rimuovo target dal canale
	channel->removeMember(targetNickname);
	channel->removeOperator(targetNickname);
	channel->removeInvited(targetNickname);

	log(LOG_SERVER, &client, "KICK executed: " + targetNickname + " from channel " + channelName);
}

void Server::handleInvite(const Message &msg, Client &client)
{
	if (msg.getParams().empty()) {
		sendMessageToClient(client.getFd(), ":" + _name + " 461 " + msg.getCommand() + " :Not enough parameters");
		log(LOG_SERVER, &client, "INVITE → 461 not enough parameters");
		return;
	}

	if (msg.getParams().size() != 2) {
		sendMessageToClient(client.getFd(), ":" + _name + " 472 " + msg.getCommand() + " :wrong number of params");
		log(LOG_SERVER, &client, "INVITE → 472 wrong number of params");
		return;
	}

	std::string targetNick = msg.getParams()[0];
	std::string channelName = msg.getParams()[1];

	if (!isValidNickname(targetNick)) {
		sendMessageToClient(client.getFd(), ":" + _name + " 432 " + client.getNickname() + " :Erroneous nickname");
		log(LOG_SERVER, &client, "INVITE → 432 invalid nickname: " + targetNick);
		return;
	}

	if (!isValidChannelName(channelName)) {
		sendMessageToClient(client.getFd(), ":" + _name + " 476 " + client.getNickname() + " :ERR_BADCHANMASK");
		log(LOG_SERVER, &client, "INVITE → 476 invalid channel name: " + channelName);
		return;
	}

	int targetFd = getFdByNickname(targetNick);
	if (targetFd == -1) {
		sendMessageToClient(client.getFd(), ":" + _name + " 401 " + client.getNickname() + " :No such nick/channel");
		log(LOG_SERVER, &client, "INVITE → 401 no such nick: " + targetNick);
		return;
	}

	Channel *ch = getChannelByName(channelName);
	if (!ch) {
		sendMessageToClient(client.getFd(), ":" + _name + " 403 " + channelName + " :No such channel");
		log(LOG_SERVER, &client, "INVITE → 403 no such channel: " + channelName);
		return;
	}

	if (!ch->isMember(client.getNickname())) {
		sendMessageToClient(client.getFd(), ":" + _name + " 442 " + channelName + " :You're not on that channel");
		log(LOG_SERVER, &client, "INVITE → 442 client not on channel: " + channelName);
		return;
	}

	if (ch->isMember(targetNick)) {
		sendMessageToClient(client.getFd(), ":" + _name + " " + targetNick + " 443 " + channelName + " :is already on channel");
		log(LOG_SERVER, &client, "INVITE → 443 target already on channel: " + targetNick);
		return;
	}

	if (ch->getInviteOnly() && !ch->isOperator(client.getNickname())) {
		sendMessageToClient(client.getFd(), ":" + _name + " " + channelName + " 482 :You're not channel operator");
		log(LOG_SERVER, &client, "INVITE → 482 not operator on invite-only channel: " + channelName);
		return;
	}

	// aggiungo come invitato
	ch->addInvited(targetNick);

	// conferma al client che ha inviato l'invito
	sendMessageToClient(client.getFd(), ":" + _name + " 341 " + client.getNickname() + " " + targetNick + " " + channelName);

	// notifico il target dell'invito
	Client &user = _clients[targetFd];
	sendMessageToClient(targetFd, ":" + client.getNickname() + "!" + client.getUsername() + "@" + client.getHostname() +
		" INVITE " + targetNick + " " + channelName);

	log(LOG_SERVER, &client, "INVITE executed: " + targetNick + " invited to " + channelName);
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

void Server::createChannel(const std::string &name)
{
	if (getChannelByName(name) != NULL)
		return ;
	_channels.push_back(Channel(name));
}

void Server::broadcastMessageToChannel(const std::string &message, const Channel &channel, const std::string &toExclude)
{
	// get Members restituisce nicknames
	// cerco gli fd per ogni nickname
	const std::set<std::string> &members = channel.getMembers();
	for (std::set<std::string>::const_iterator it = members.begin(); it != members.end(); ++it) {
			if (*it != toExclude) {
					int fd = getFdByNickname(*it);
					if (fd != -1)
							sendMessageToClient(fd, message);
			}
	}
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
	logOut(_clients[fd], message);
	std::string msgWithCRLF = message + "\r\n";
	const char *data = msgWithCRLF.c_str();
	size_t total = msgWithCRLF.size();
	size_t sent = 0;
	while (sent < total) {
		ssize_t n = send(fd, data + sent, total - sent, 0);
		if (n < 0) {
			if (errno == EINTR)
			continue;
			log(LOG_SERVER, &_clients[fd], "send() error");
			size_t idx = pollfdIndexByFd(fd);
			if (idx < _pollFds.size())
				handleClientDisconnection(idx);
			return ;
		}
			if (n == 0) // connessione chiusa dall'altro lato
				break;
			sent += n;
		}
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

	sendMessageToClient(client.getFd(), ":" + _name + " 001 " + nick + " :Welcome to the IRC network, " + nick + "!");
	sendMessageToClient(client.getFd(), ":" + _name + " 002 " + nick + " :Your host is " + _name + ", running version 1.0");
	sendMessageToClient(client.getFd(), ":" + _name + " 003 " + nick + " :This server was created " + std::string(__DATE__));
	sendMessageToClient(client.getFd(), ":" + _name + " 004 " + nick + " " + _name + " 1.0 o o");
	log(LOG_SERVER, &client, "Sent welcome messages (001-004)");
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

/*
	Non può iniziare con un numero
	Non può iniziare con # o & (sono prefissi di canale) o con : (è il prefisso dei messaggi IRC)
	Non può contenere spazi, \r \n  ,  *  ? (sono caratteri speciali del protocollo)
	Non può essere vuoto
*/
bool Server::isValidNickname(const std::string &nickname) {
	if (nickname.empty() || nickname.length() > 9)
		return false;
	if (std::isdigit(nickname[0]) || nickname[0] == '#' || nickname[0] == '&' || nickname[0] == ':')
		return false;
	for (size_t i = 0; i < nickname.length(); i++) {
		if (isspace(nickname[i]) || nickname[i] == ',' || nickname[i] == '*'
			|| nickname[i] == '?' || nickname[i] == '\r' || nickname[i] == '\n')
			return false;
	}
	return true;
}

// la lunghezza in realta' è 200 caratteri incluso prefisso
// Non può essere solo # o & senza niente dopo
// Non può includere '\x07' - ASCII BEL (bell sound)
bool Server::isValidChannelName(const std::string &channelName)
{
	if (channelName.length() > 100 || channelName.length() < 2)
		return false;
	if (channelName[0] != '#' && channelName[0] != '&')
		return false;
	for (size_t i = 1; i < channelName.length(); i++) {
		if (isspace(channelName[i]) || channelName[i] == ',' || channelName[i] == ':'
			|| channelName[i] == '\r' || channelName[i] == '\n' || channelName[i] == '\x07')
			return false;
	}
	return true;
}

#endif
