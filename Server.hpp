/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 21:25:20 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/22 17:03:24 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "includes.hpp"
# include "Client.hpp"

class Server {
	private:
		int							_port;
		std::string					_password; // PASS mypassword -> mypassword == _passoword, altrimenti connessione rifiutata

		int							_serverFd; // socket del server (ascolta nuovi client)
		std::vector<struct pollfd>	_fds;	   // array di fd dei clients da monitorare
		std::map<int, Client>		_clients;  // associazione fd -> oggetto client
	public:
		Server(const int port, const std::string &password);
		~Server();

		void	run();
		static bool	isValidPort(const std::string &port);
		static bool	isValidPassword(const std::string &password);
};

//struct pollfd è una struttura dati del sistema operativo che contiene il file descriptor da osservare,
//la maschera degli eventi di interesse e la maschera degli eventi che si sono verificati.
//Viene passata a poll() come array per il monitoraggio simultaneo di più fd.
// struct pollfd {
//     int   fd;       // quale fd monitorare
//     short events;   // cosa vuoi sapere (es. POLLIN)
//     short revents;  // cosa è successo (lo riempie poll())
// };

#endif