/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 17:56:28 by plichota          #+#    #+#             */
/*   Updated: 2026/03/25 13:23:23 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(std::string name) : _name(name), _topic(""), _password(""), _modes(), _operators(), _clients(), _maxUsers(-1)
{
}

Channel::~Channel()
{

}

std::string Channel::getName()
{
    return this->_name;
}

void Channel::printChannelInfo()
{
	std::cout << "Channel: " << this->_name
		<< ", Topic: " << this->_topic
		<< ", Password: " << this->_password
		<< ", client count: " << this->_clients.size()
		<< ", operator count: " << this->_operators.size()
		<< std::endl;
}