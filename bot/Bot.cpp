/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/26 22:00:29 by plichota          #+#    #+#             */
/*   Updated: 2026/03/27 18:17:37 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include "signal.hpp"
#include <iostream>
#include <cstdlib> // atoi
#include <unistd.h> // close()

#include <sys/socket.h> // socket()
#include <netinet/in.h> // sockaddr_in
#include <cstring> // memset()
#include <arpa/inet.h> // inet_pton()  -> pton sta per "presentation to network"

Bot::Bot(int port, const std::string &password, const std::string &channel) :
    _name("LouBottin"), _fd(-1), _port(port), _password(password),
    _channel(channel), _warnedNicknames(), _fobiddenWords()
{
    initForbiddenWords();
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

void Bot::initForbiddenWords()
{
    _fobiddenWords.push_back("mela");
    _fobiddenWords.push_back("pera");
    _fobiddenWords.push_back("zio");
}

void Bot::updateNickname(const std::string &newNickname, const std::string &oldNickname)
{
    for (size_t i = 0; i < _warnedNicknames.size(); i++) {
        if (_warnedNicknames[i] == oldNickname) {
            _warnedNicknames[i] = newNickname;
            return;
        }
    }
}

void Bot::joinChannel()
{
    sendMessage("JOIN " + _channel);
}

void Bot::registerClient()
{
    sendMessage("PASS " + _password);
    sendMessage("NICK " + _name);
    sendMessage("USER " + _name + " 0 *");
}

void Bot::kickUser(const std::string &nick)
{
    sendMessage("KICK " + _channel + " " + nick);
}

void Bot::sendMessage(const std::string &message)
{
    std::string full = message + "\r\n";
    send(_fd, full.c_str(), full.size(), 0);
    std::cout << "> " << message << std::endl;
}

void Bot::sendMessageToChannel(const std::string &message)
{
    std::string msg = "PRIVMSG " + _channel + " :" + message + "\r\n";
    send(_fd, msg.c_str(), msg.size(), 0); // aggiungi PRIVMSG _channel
    std::cout << "> " << message << std::endl;
}

bool Bot::isWarned(const std::string &nick)
{
    for (size_t i = 0; i < _warnedNicknames.size(); i++) {
        if (_warnedNicknames[i] == nick)
            return true;
    }
    return false;
}

void Bot::addWarned(const std::string &nick)
{
    _warnedNicknames.push_back(nick);
}

void Bot::removeWarned(const std::string &nick)
{
    for (size_t i = 0; i < _warnedNicknames.size(); i++) {
        if (_warnedNicknames[i] == nick) {
            _warnedNicknames.erase(_warnedNicknames.begin() + i);
            return;
        }
    }
}

void Bot::handlePong(const std::string &line)
{
    sendMessage("PONG " + line.substr(5));
    return;
}

// :vecchionick!user@host NICK :nuovonick
void Bot::handleNick(const std::string &line)
{
    std::string oldNick = line.substr(1, line.find('!') - 1);
    std::string::size_type pos = line.rfind(':');
    std::string newNick = line.substr(pos + 1);
    updateNickname(oldNick, newNick);
    return;
}

// :nick!user@host JOIN #canale
void Bot::handleJoin(const std::string &line)
{
    std::string nick = line.substr(1, line.find('!') - 1);
    if (nick != _name)
        sendMessageToChannel("Welcome " + nick + " 𒆜𓊉꧂");
}

bool Bot::hasForbiddenWord(const std::string &message)
{
    std::string lower = message;
    for (size_t i = 0; i < lower.size(); i++)
        lower[i] = std::tolower(lower[i]);

    for (size_t i = 0; i < _fobiddenWords.size(); i++) {
        if (message.find(_fobiddenWords[i]) != std::string::npos)
            return true;
    }
    return false;
}

// :nick!user@127.0.0.1 PRIVMSG #42 :ciao a tutti
void Bot::handlePrivmsg(const std::string &line)
{
    std::string nick = line.substr(1, line.find('!') - 1);
    std::string message = line.substr(line.find(':') + 1);
    std::cout << message << std::endl;
    if (hasForbiddenWord(message))
    {
        if (isWarned(nick))
            kickUser(nick);
        else
        {
            sendMessageToChannel("Warning: " + nick + " used a forbidden word 🕱");
            _warnedNicknames.push_back(nick);
        }
    }
}

void Bot::handleLine(const std::string &line)
{
    std::cout << "> " << line << std::endl;
    if (line.find(" PING ") != std::string::npos)
        handlePong(line);
    else if (line.find(" NICK ") != std::string::npos)
        handleNick(line);
    else if (line.find(" JOIN ") != std::string::npos)
        handleJoin(line);
    else if (line.find(" PRIVMSG ") != std::string::npos)
        handlePrivmsg(line);
    else if (line.find("464 ") != std::string::npos)
    {
        std::cerr << "Wrong password" << std::endl;
        throw std::runtime_error("Wrong password");
    }
    else if (line.find(" 433 ") != std::string::npos)  // nick del bot gia' in uso
    {
        std::cerr << "Nickname already in use" << std::endl;
        throw std::runtime_error("Nickname already in use");
    }
}

void Bot::handleLoop()
{
    char temp[1024];
    std::string buffer;

    while (received_signal == 0)
    {
        int bytes = recv(_fd, temp, sizeof(temp) - 1, 0); // -1 per lo \0
        if (bytes <= 0) // connessione chiusa o errore
            break;
        temp[bytes] = '\0'; // per usare buffer come stringa
        buffer += temp;

        std::string::size_type pos;
        while ((pos = buffer.find("\r\n")) != std::string::npos) // cerco fine messaggio
        {
            std::string line = buffer.substr(0, pos);
            buffer.erase(0, pos + 2); // compreso \r\n
            handleLine(line);
        }
    }
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

    registerClient();
    joinChannel();
    handleLoop();
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
