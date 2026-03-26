/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: francema <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 17:18:37 by plichota          #+#    #+#             */
/*   Updated: 2026/03/26 11:31:59 by francema         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

Message::Message() : _prefix(""), _command(""), _params(), _hasTrailing(false),_trailing(""), _valid(true), _error("") {}

Message::Message(const Message &other)
	:_prefix(other._prefix),
	_command(other._command),
	_params(other._params),
	_hasTrailing(other._hasTrailing),
	_trailing(other._trailing),
	_valid(other._valid),
	_error(other._error)
	{}

Message &Message::operator=(const Message &other) {
	if (this != &other) {
		_prefix  = other._prefix;
		_command = other._command;
		_params  = other._params;
		_hasTrailing = other._hasTrailing;
		_trailing = other._trailing;
		_valid = other._valid;
		_error = other._error;
	}
	return *this;
}

Message::~Message() {}

Message::Message(const std::string &raw): _prefix(""), _command(""), _params(), _hasTrailing(false), _trailing(""), _valid(true), _error("") {
	std::string line = raw;

	//trim leading spaces
	size_t start = line.find_first_not_of(' ');
	if (start != std::string::npos)
		line = line.substr(start);
	else
		line.clear();

	// 1. prefix extraction
	if (!line.empty() && line[0] == ':') {
		size_t space = line.find(' ');
		if (space == std::string::npos ){
			_valid = false;
			_error = "461";
			return ;
		}
		_prefix = line.substr(1, space - 1);
		line = line.substr(space + 1); //taglia fino all'inizio del cmd; restituisce da cmd in poi
	}

	//trim leading spaces after prefix
	start = line.find_first_not_of(' ');
	if (start != std::string::npos)
		line = line.substr(start);
	else
		line.clear();

	// 2. command extraction
	size_t space = line.find(' ');
	if (space == std::string::npos) {
		if (line.empty()) {
			_valid = false;
			_error = "421";
			return;
		}
		_command = line;
		return;
	}
	_command = line.substr(0, space);
	line = line.substr(space + 1); // taglia il cmd da line e lascia tutto il resto

	// 3. params and trailing extraction
	while (!line.empty()) {
		if (line[0] == ':') {
			_hasTrailing = true;
			_trailing = line.substr(1);
			break;
		}
		if (line[0] == ' ') {
			line = line.substr(1);
			continue;
		}
		size_t next = line.find(' ');
		if (next == std::string::npos) {
			_params.push_back(line);
			break;
		}
		_params.push_back(line.substr(0, next)); // mette param per param tutto dentro il vettore
		line = line.substr(next + 1); //taglia il param inserito restituendo il resto
	}
}

/* -------------------------------------------------------------------------- */

const std::string&	Message::getPrefix() const { return _prefix; }

const std::string&	Message::getCommand() const { return _command; }

const std::vector<std::string>&	Message::getParams() const { return _params; }

const std::string&	Message::getError() const { return _error; }

bool Message::hasTrailing() const { return _hasTrailing; }

const std::string&	Message::getTrailing() const { return _trailing; }

bool	Message::isValid() const { return _valid; }
