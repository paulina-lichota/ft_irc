/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: francema <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 21:25:20 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/27 10:00:54 by francema         ###   ########.fr       */
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
		// void log(LogType type, const Client* client, const std::string& msg);
		// void errlog(Client &client, const std::string &code, const std::string &target, const std::string &message);
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

#endif
