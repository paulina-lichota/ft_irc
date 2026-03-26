/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/26 22:00:29 by plichota          #+#    #+#             */
/*   Updated: 2026/03/26 22:22:54 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include <iostream>
#include <cstdlib> // atoi
#include <unistd.h> // close()

Bot::Bot(int port, const std::string &password, const std::string &channel) : _name("LouBottin"), _fd(-1), _port(port), _password(password), _channel(channel)
{
}

Bot::~Bot()
{
    if (_fd != -1)
        close(_fd);
}

void Bot::run()
{
    std::cout << "Bot is running on port " << _port << ", joining channel " << _channel << std::endl;
}

// Le porte 1-1024 sono riservate al sistema
// il range valido è 1025-65535 (16 bit nel protocollo TCP)
bool Bot::isValidPort(const std::string &port)
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
