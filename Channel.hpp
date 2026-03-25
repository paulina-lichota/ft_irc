/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cwannhed <cwannhed@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 17:55:39 by plichota          #+#    #+#             */
/*   Updated: 2026/03/25 10:42:04 by cwannhed         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "includes.hpp"
#include "Client.hpp"

/*
	All the messages sent from one client to a channel have to be forwarded to every other client that joined the channel.
*/
class Channel
{
	private:
		std::string _name;
		std::string _topic;
		std::string _password; // optional -k
		std::set<char> _modes; // {i, t, k, o, l} the value itself acts as a key
		/*
			· i: Set/remove Invite-only channel
			· t: Set/remove the restrictions of the TOPIC command to channel operators
			· k: Set/remove the channel key (password)
			· o: Give/take channel operator privilege
			· l: Set/remove the user limit to channel
		*/
		std::vector<Client> _clients; // lista di tutti i client connessi al canale
		std::set<int> _operators;	  // fd degli operatori
		int _maxUsers;

	public:
		Channel(std::string name);
		~Channel();

		// Getters
		std::string getName();

		// Everyone
		void propagateMessageToChannel(std::string message, Client sender); // invia message a tutti i client del canale tranne sender

		// Operators
		void kick(Client client);		  // kick un client dal canale
		void invite(Client client);		  // invita un client al canale
		void setTopic(std::string topic); // setta il topic del canale
		void setMode(char mode);		  // setta un mode del canale // +- operatore
};

#endif