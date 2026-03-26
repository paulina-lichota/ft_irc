/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/26 21:47:20 by plichota          #+#    #+#             */
/*   Updated: 2026/03/26 22:50:45 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOT_HPP
#define BOT_HPP

#include <string>

class Bot
{
    private:
        std::string _name;
        int _fd;
        int _port;
        std::string _password;
        std::string _channel;
    public:
        Bot(int port, const std::string &password, const std::string &channel);
        ~Bot();

        static bool isValidPort(const std::string &port);
        struct sockaddr_in initAddStruct(int port);

        void run();
};

#endif