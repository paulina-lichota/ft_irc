/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 21:25:20 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/21 21:39:29 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "includes.hpp"

class Server {
private:
	int			_port;
	std::string	_password;
public:
	Server(const int port, const std::string &password);
	~Server();

	void	run();
	static bool	isValidPort(const std::string &port);
	static bool	isValidPassword(const std::string &password);
};




#endif