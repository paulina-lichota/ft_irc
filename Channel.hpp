/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 17:55:39 by plichota          #+#    #+#             */
/*   Updated: 2026/03/25 15:27:44 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include "includes.hpp"
#include "Client.hpp"

/*
	· i: Set/remove Invite-only channel
	· t: Set/remove the restrictions of the TOPIC command to channel operators
	· k: Set/remove the channel key (password)
	· o: Give/take channel operator privilege
	· l: Set/remove the user limit to channel
*/
class Channel
{
	private:
		const std::string	_name; // immutabile
		std::string	_topic;
		std::string	_key; 	// empty = no password
		bool	_inviteOnly;	// +i: solo operatori possono invitare nuovi client, se un client prova ad unirsi senza essere stato invitato, ERR 473
		bool	_topicRestricted; //  +t: solo operatori possono cambiare topic (ERR 482 se membro non operatore ci prova)
		size_t		_usersLimit; // 0 = no limit, modificabile con MODE +l
		std::set<std::string> _operators;	  // +o = proprietà degli utenti nicknames degli operatori (no fd perchè possono essere riassegnati)
		std::set<std::string> _members;	  // nicknames di tutti i client connessi al canale (inclusi operatori)
		std::set<std::string> _invited;	  // nicknames di tutti i client invitati

	public:
		Channel(const std::string &name);
		~Channel();

		// Info e modes
		const std::string &getName() const; // name immutable, only getter

		const std::string &getKey() const;
		void setKey(const std::string &key); // only op, se key è empty, rimuove la password

		bool getTopicRestricted() const;
		void setTopicRestricted(bool restricted); // only op
	
		const std::string &getTopic() const;
		void setTopic(const std::string& topic); // if _topicRestricted, only op

		bool getInviteOnly() const;
		void setInviteOnly(bool inviteOnly); // only op
		
		size_t getUsersLimit() const;
		void setUsersLimit(size_t limit); // only op

		// Membri
		void addMember(const std::string &nickname);
		void removeMember(const std::string &nickname);
		bool isMember(const std::string &nickname) const;
		size_t getMemberCount() const;
		const std::set<std::string> &getMembers() const;

		// Operatori
		void addOperator(const std::string &nickname); // only op, or first member that creates the channel
		void removeOperator(const std::string &nickname); // only op
		bool isOperator(const std::string &nickname) const; 
		size_t getOperatorCount() const;
		const std::set<std::string> &getOperators() const;

		// Inviti
		bool isInvited(const std::string &nickname) const;
		void addInvited(const std::string &nickname); // if _inviteOnly, only op can add invited
		void removeInvited(const std::string &nickname); // rimuovi dopo il JOIN

		void printChannelInfo();
};

#endif