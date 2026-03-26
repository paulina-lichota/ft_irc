/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/26 22:00:15 by plichota          #+#    #+#             */
/*   Updated: 2026/03/26 22:08:38 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include <iostream>
#include <sstream>

// Le porte 1-1024 sono riservate al sistema
// il range valido è 1025-65535 (16 bit nel protocollo TCP)
bool isValidPort(const std::string &port)
{
    if (port.empty())
        return false;
    for (size_t i = 0; i < port.size(); i++) {
        if (!std::isdigit(port[i]))
            return false;
    }
    int p = std::atoi(port.c_str());
    return p > 1024 && p <= 65535;
}

// ./bot <port> <password> <channel>
int main(int argc, char **argv)
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <port> <password> <channel>" << std::endl;
        return (1);
    }
    try
    {
        // port non e' un numero
        if (!isValidPort(argv[1]))
            throw std::invalid_argument("Error: invalid port.");

        Bot bot(std::atoi(argv[1]), argv[2], argv[3]);
        bot.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return (1);
    }
    return (0);
}