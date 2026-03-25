/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 16:23:34 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/25 12:52:33 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "includes.hpp"

class Client {
	private:
		int			_fd;
		std::string	_nickname; //nickname scelto dal client, deve essere unico (NICK <nickname>)
		std::string	_username; //settato con USER <username> <hostname> <servername> <realname>, usato per identificare il client, non appare nei messaggi, usato per i prefix dei messaggi (es. :nickname!username@hostname PRIVMSG #channel :message)
		std::string	_realname; // ''
		std::string	_hostname; // host or IP address
		std::string	_buffer; // buffer per dati ricevuti dal client, usato per gestire messaggi incompleti
		bool		_passwordAccepted; // true se il client ha inviato PASS <password> corretta, altrimenti false
		bool		_registered; // true se il client ha completato la registrazione (PASS + NICK + USER), altrimenti false
	public:
		Client();
		Client(int fd, const std::string &hostname);
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();

		// Getters
		std::string &getBuffer();
		int getFd() const;
		bool getPasswordAccepted() const;
		bool getRegistered() const;
		const std::string& getNickname() const;
		const std::string& getUsername() const;
		const std::string& getHostname() const;
		// Setters
		void setNickname(const std::string &nickname);
		void setUsername(const std::string &username);
		void setRealname(const std::string &realname);
		void setHostname(const std::string &hostname);
		void setRegistered(bool registered);
		void setPasswordAccepted(bool accepted);

		// Message
		void appendToBuffer(const std::string &data);
		std::string extractMessageFromBuffer(); // estrae un messaggio completo dal buffer, se presente, e lo rimuove dal buffer
};

#endif
