/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 16:23:34 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/23 19:11:59 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "includes.hpp"

class Client {
private:
	int			_fd;
	//nickname
	std::string	_buffer;

public:
	Client();
	Client(int fd);
	~Client();

	std::string &getBuffer();
	int getFd() const;
	void appendToBuffer(const std::string &data);
};

#endif