/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/26 22:00:29 by plichota          #+#    #+#             */
/*   Updated: 2026/03/26 22:15:09 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include <iostream>

Bot::Bot(int port, const std::string &password, const std::string &channel) : _name("LouBottin"), _fd(-1), _port(port), _password(password), _channel(channel)
{
}

Bot::~Bot() {}

void Bot::run()
{
    std::cout << "Hello, world!" << std::endl;
}