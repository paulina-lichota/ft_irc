/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/26 21:47:20 by plichota          #+#    #+#             */
/*   Updated: 2026/03/27 15:05:19 by plichota         ###   ########.fr       */
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
        std::vector<std::string> _warnedNicknames;
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

        void updateNickname(const std::string &newNickname, const std::string &oldNickname);
        void kickUser(const std::string &nick);

        bool isWarned(const std::string &nick);
        void addWarned(const std::string &nick);
        void removeWarned(const std::string &nick);

        void handlePong(const std::string &line);
        void handleNick(const std::string &line);
        void handleJoin(const std::string &line);
        void handlePrivmsg(const std::string &line);

        void handleLine(const std::string &line);

        bool hasForbiddenWord(const std::string &message);
        static bool isValidPort(const std::string &port);
        struct sockaddr_in initAddStruct(int port);
};

#endif