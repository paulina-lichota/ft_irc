/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/26 21:47:20 by plichota          #+#    #+#             */
/*   Updated: 2026/03/26 23:38:19 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOT_HPP
#define BOT_HPP

#include <string>
#include <vector>

class Bot
{
    private:
        std::string _name;
        int _fd;
        int _port;
        std::string _password;
        std::string _channel;
        std::vector<std::string> _fobiddenWords;
    public:
        Bot(int port, const std::string &password, const std::string &channel);
        ~Bot();

        void initForbiddenWords();

        void run();
        void joinChannel();
        void registerClient();
        void sendMessage(const std::string &message);
        void handleLoop();

        static bool isValidPort(const std::string &port);
        struct sockaddr_in initAddStruct(int port);
};

#endif