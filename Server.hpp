/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 21:25:20 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/23 19:11:02 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "includes.hpp"
#include "Client.hpp"

class Server {
private:
	int							_port;
	std::string					_password; // PASS mypassword -> mypassword == _passoword, altrimenti connessione rifiutata
	int							_serverFd; // socket del server (ascolta nuovi client)
	std::vector<struct pollfd>	_pollFds; // array di fd dei clients da monitorare
	std::map<int, Client>		_clients;	 // associazione fd -> oggetto client
	static const int			POLL_TIMEOUT = -1; // -1 per aspettare eventi all'infinito
public:
	Server(const int port, const std::string &password);
	~Server();

	void run();
	void addPollFd(int fd);
	void handleNewConnection();
	void handleClientDisconnection(size_t index);
	static bool isValidPort(const std::string &port);
	static bool isValidPassword(const std::string &password);
};

// struct pollfd è una struttura dati del sistema operativo che contiene il file descriptor da osservare,
// la maschera degli eventi di interesse e la maschera degli eventi che si sono verificati.
// Viene passata a poll() come array per il monitoraggio simultaneo di più fd.
// struct pollfd {
//     int   fd;       // quale fd monitorare
//     short events;   // cosa vuoi sapere (es. POLLIN)
//     short revents;  // cosa è successo (lo riempie poll())
// };

/*
	I valori principali:

	POLLIN → ci sono dati da leggere (o una nuova connessione sul server socket)
	POLLOUT → puoi scrivere senza bloccarti
	POLLHUP → il client si è disconnesso
	POLLERR → errore sul fd
*/

#endif