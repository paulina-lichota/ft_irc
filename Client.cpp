/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 16:24:21 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/24 15:05:49 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() : _fd(-1),
	_nickname(""),
	_username(""),
	_realname(""),
	_hostname(""),
	_buffer("")
	// _passwordAccepted(false),
	//_registered(false)
	{}

Client::Client(int fd, const std::string &hostname) : _fd(fd),
	_nickname(""),
	_username(""),
	_realname(""),
	_hostname(hostname),
	_buffer("")
	// _passwordAccepted(false),
	//_registered(false)
	{}

Client::~Client() {}

/* -------------------------------------------------------------------------- */

std::string &Client::getBuffer() {
	return (_buffer);
}

int Client::getFd() const {
	return (_fd);
}

void Client::appendToBuffer(const std::string &data) {
	_buffer += data;
}

std::string Client::extractMessageFromBuffer() {
	size_t pos = _buffer.find("\r\n"); // i messaggi IRC terminano con \r\n
	if (pos == std::string::npos)
		return (""); // non c'è un messaggio completo nel buffer
	std::string message = _buffer.substr(0, pos); // estraggo messaggio completo
	_buffer.erase(0, pos + 2); // rimuovo messaggio estratto dal buffer
	return (message);
}
