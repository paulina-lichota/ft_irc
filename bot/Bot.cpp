/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/26 22:00:29 by plichota          #+#    #+#             */
/*   Updated: 2026/03/26 22:54:14 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include <iostream>
#include <cstdlib> // atoi
#include <unistd.h> // close()

#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in
#include <cstring> // memset()
#include <arpa/inet.h> // inet_pton()  -> pton sta per "presentation to network"

Bot::Bot(int port, const std::string &password, const std::string &channel) : _name("LouBottin"), _fd(-1), _port(port), _password(password), _channel(channel)
{
}

Bot::~Bot()
{
    if (_fd != -1)
        close(_fd);
}


// struct sockaddr_in {
//     short            sin_family;   // e.g., AF_INET
//     unsigned short   sin_port;     // Port number (must be htons())
//     struct in_addr   sin_addr;     // IPv4 address (must be inet_aton())
//     char             sin_zero[8];  // Zero this if you want to
// };
struct sockaddr_in Bot::initAddStruct(int port)
{
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; // IPv4
    addr.sin_port = htons(port); // porta, convertita in network byte order (al contrario essenzialmente)
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr); // converte in binario (senno' kernel non capisce)
    return (addr);
}

void Bot::run()
{
    // STEP 1: creo socket
    _fd = socket(AF_INET, SOCK_STREAM, 0); // IPV4, TCP, kernel sceglie protocollo
    if (_fd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return ;
    }

    struct sockaddr_in addr = initAddStruct(_port);

    // STEP 2: connect al server
    if (connect(_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        std::cerr << "Error connecting to server" << std::endl;
        close(_fd);
        _fd = -1;
        return ;
    }

    std::cout << "Bot is running on port " << _port << std::endl;
    
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
