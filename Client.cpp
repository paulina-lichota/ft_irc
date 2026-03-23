/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 16:24:21 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/23 18:49:07 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client() : _fd(-1), _buffer("") {}

Client::Client(int fd) : _fd(fd), _buffer("") {}

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
