/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 17:55:39 by plichota          #+#    #+#             */
/*   Updated: 2026/03/24 18:30:39 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <set>
#include "Client.hpp"

/*
    All the messages sent from one client to a channel have to be forwarded to every other client that joined the channel.
*/
class Channel
{
    private:
        std::string name;
        int user_limit; // -l
        std::vector<Client> clients; // lista di tutti i client connessi al canale
        std::set<int> operators; // fd degli operatori
    public:
        Channel(std::string);
        ~Channel();
        std::string getName();

        void propagateMessageToChannel(std::string message, Client sender); // invia message a tutti i client del canale tranne sender
};

#endif