/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 21:26:11 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/22 20:03:18 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(const int port, const std::string &password) : _port(port), _password(password) {
	//setup server

	//creo socket (fd) (unico scopo: accettare nuove connessioni)
	//socket() alloca un nuovo socket nel kernel e ritorna il file descriptor
	//setsockopt() con SO_REUSEADDR permette il riuso immediato della porta (latirmenti rimane bloccata per 60 sec)
	//associo socket - indirizzo locale (ip + port) con bind()
	//metto il socket in stato listen ()
	//server pronto ad ascoltare
}

Server::~Server() {}

/* -------------------------------------------------------------------------- */

//The server must be capable of handling multiple clients simultaneously without hanging.
// -> poll() ascolta eventi sui file descriptor

// Forking is prohibited. All I/O operations must be non-blocking.
// poll() + recv() (in questo ordine)
// recv() legge i dati arrivati su un socket e li mette in un buffer

// Loop di ascolto dei client
void	Server::run(){

	//loop con poll() che aspetta eventi sugli fd aperti (multiplexing)
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
