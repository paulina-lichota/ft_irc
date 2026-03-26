/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 17:18:44 by plichota          #+#    #+#             */
/*   Updated: 2026/03/26 01:40:53 by plichota         ###   ########.fr       */
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
	bool                  _hasTrailing;
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
	bool							isValid()		const;
	const std::string&				getError()		const;
	const std::string&				getTrailing()	const;
	bool 						hasTrailing()	const;
};

#endif
