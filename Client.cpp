/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 16:24:21 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/25 19:10:40 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() : _fd(-1),
	_nickname(""),
	_username(""),
	_realname(""),
	_hostname(""),
	_buffer(""),
	_passwordAccepted(false),
	_registered(false)
	{}

Client::Client(int fd, const std::string &hostname) : _fd(fd),
	_nickname(""),
	_username(""),
	_realname(""),
	_hostname(hostname),
	_buffer(""),
	_passwordAccepted(false),
	_registered(false)
	{}

Client::Client(const Client& other): _fd(other._fd),
	_nickname(other._nickname),
	_username(other._username),
	_realname(other._realname),
	_hostname(other._hostname),
	_buffer(other._buffer),
	_passwordAccepted(other._passwordAccepted),
	_registered(other._registered)
	{}

Client& Client::operator=(const Client& other){
	if (this != &other) {
		_fd = (other._fd);
		_nickname = (other._nickname);
		_username = (other._username);
		_realname = (other._realname);
		_hostname = (other._hostname);
		_buffer = (other._buffer);
		_passwordAccepted = (other._passwordAccepted);
		_registered = (other._registered);
	}
	return *this;
}

Client::~Client() {}

/* ------------------------------------ Getters ----------------------------------- */

std::string &Client::getBuffer() { return (_buffer); }

int Client::getFd() const { return (_fd); }

bool Client::getPasswordAccepted() const { return (_passwordAccepted); }

bool Client::getRegistered() const { return (_registered); }

const std::string& Client::getNickname() const { return (_nickname); }

const std::string& Client::getUsername() const { return (_username); }

const std::string& Client::getHostname() const { return (_hostname); }

/* ------------------------------------ Setters ----------------------------------- */

void Client::setNickname(const std::string &nickname) { _nickname = nickname; }

void Client::setUsername(const std::string &username) { _username = username; }

void Client::setPasswordAccepted(bool accepted) { _passwordAccepted = accepted; }

void Client::setRegistered(bool registered) { _registered = registered; }

void Client::setRealname(const std::string &realname) { _realname = realname;}

void Client::setHostname(const std::string &hostname) { _hostname = hostname; }

/* -------------------------------------------------------------------------- */

void Client::appendToBuffer(const std::string &data) {
	_buffer += data;
}

/*
** Estrae un messaggio completo dal buffer di ricezione del client.
**
** I messaggi IRC terminano con \r\n — finché il delimitatore non è presente,
** il messaggio è incompleto (TCP può consegnare i dati in frammenti).
**
** Se \r\n è trovato: estrae la stringa fino al delimitatore, rimuove
** il messaggio estratto dal buffer (incluso \r\n), e lo ritorna.
** Se \r\n non è trovato: ritorna stringa vuota — il chiamante richiamerà
** questa funzione al prossimo recv().
*/
std::string Client::extractMessageFromBuffer() {
	size_t pos = _buffer.find("\r\n");
	if (pos == std::string::npos)
		return ("");
	std::string message = _buffer.substr(0, pos);
	_buffer.erase(0, pos + 2);
	return (message);
}
