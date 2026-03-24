/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 16:32:09 by cwannhed          #+#    #+#             */
/*   Updated: 2026/03/24 16:54:17 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MESSAGE_HPP
# define MESSAGE_HPP

# include "includes.hpp"

class Message {
private:
	std::string					_prefix;
	std::string					_command;
	std::vector<std::string>	_params;
	std::string					_trailing; // parte finale del messaggio dopo i parametri, inizia con ':' e può contenere spazi, es. :message text with spaces
	bool						_valid;
	std::string					_error;
public:
	Message();
	Message(const std::string &raw);
	Message(const Message& other);
	Message& operator=(const Message& other);
	~Message();

	const std::string&				getPrefix()		const;
	const std::string&				getCommand()	const;
	const std::vector<std::string>&	getParams()		const;
	const std::string&				getTrailing()	const;
};

#endif
