/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: francema <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/20 15:28:50 by plichota          #+#    #+#             */
/*   Updated: 2026/03/24 17:03:48 by francema         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes.hpp"
#include "Server.hpp"

// ./ircserv <port> <password>
int main(int argc, char **argv) {
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
