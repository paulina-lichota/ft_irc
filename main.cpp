/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2026/03/27 14:17:37 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "includes.hpp"
#include "signal.hpp"
#include "Server.hpp"

// ./ircserv 6667 password

// usare ip non localhost per file transfer
// ping a google
// nc -C 127.0.0.1 6667
//  -C usa \r\n come separatore

/*
 se non va Ctrl+D usare sleep:

{
    printf "PASS "
    sleep 0.5
    printf "pass"
    sleep 0.5
    printf "\r\n"
    sleep 0.5
    printf "NICK paolo\r\n"
    sleep 0.5
    printf "USER paolo 0 * :Paolo\r\n"
    sleep 10
} | nc -C 127.0.0.1 6667
*/

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
