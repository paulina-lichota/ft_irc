/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: francema <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 16:24:21 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/24 16:19:47 by francema         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() : _fd(-1),
	_nickname(""),
	_username(""),
	_realname(""),
	_hostname(""),
	_buffer("") {}
//	_passwordAccepted(false),
//	_registered(false) {}

Client::Client(int fd, const std::string &hostname) : _fd(fd),
	_nickname(""),
	_username(""),
	_realname(""),
	_hostname(hostname),
	_buffer("") {}
	// _passwordAccepted(false),
	// _registered(false)


Client::Client(const Client& other): _fd(other._fd),
	_nickname(other._nickname),
	_username(other._username),
	_realname(other._realname),
	_hostname(other._hostname),
	_buffer(other._buffer) {}
//	_passwordAccepted(other._passwordAccepted),
//	_registered(other._registered)


Client& Client::operator=(const Client& other){
	if (this != &other) {
		_fd = (other._fd);
		_nickname = (other._nickname);
		_username = (other._username);
		_realname = (other._realname);
		_hostname = (other._hostname);
		_buffer = (other._buffer);
//		_passwordAccepted = (other._passwordAccepted);
//		_registered = (other._registered);
	}
	return *this;
}

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
