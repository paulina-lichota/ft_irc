/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: francema <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2026/03/26 11:52:09 by francema         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "includes.hpp"

#include "signal.hpp"
#include "Server.hpp"

// ./ircserv 6667 password
// nc -C 127.0.0.1 6667

int main(int argc, char **argv)
{
	init_signals();
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return (1);
	}
	try {
		if (!Server::isValidPort(argv[1]))
			throw std::invalid_argument("Error: invalid port.");
		if (!Server::isValidPassword(argv[2]))
			throw std::invalid_argument("Error: invalid password.");
		Server server(std::atoi(argv[1]), argv[2]);
		server.run();
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		return (1);
	}
	return (0);
}
