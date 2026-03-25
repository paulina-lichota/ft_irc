/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: plichota <plichota@student.42firenze.it    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/24 17:56:28 by plichota          #+#    #+#             */
/*   Updated: 2026/03/25 15:27:36 by plichota         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(const std::string &name): 
    _name(name),
    _topic(""),
    _key(""),
    _inviteOnly(false),
    _topicRestricted(false),
    _usersLimit(0),
    _operators(),
    _members(),
    _invited()
{}

Channel::~Channel()
{}

/* ------------------------------------ Info & Modes ----------------------------------- */

const std::string &Channel::getName() const
{
    return this->_name;
}

const std::string &Channel::getKey() const
{
    return this->_key;
}

void Channel::setKey(const std::string &key)
{
    this->_key = key;
}

bool Channel::getTopicRestricted() const
{
    return this->_topicRestricted;
}

void Channel::setTopicRestricted(bool restricted)
{
    this->_topicRestricted = restricted;
}

const std::string &Channel::getTopic() const
{
    return this->_topic;
}

void Channel::setTopic(const std::string& topic)
{
    this->_topic = topic;
}

bool Channel::getInviteOnly() const
{
    return this->_inviteOnly;
}

void Channel::setInviteOnly(bool inviteOnly)
{
    this->_inviteOnly = inviteOnly;
}

size_t Channel::getUsersLimit() const
{
    return this->_usersLimit;
}

void Channel::setUsersLimit(size_t limit)
{
    this->_usersLimit = limit;
}

/* ------------------------------------ Members ----------------------------------- */

void Channel::addMember(const std::string &nickname)
{
    this->_members.insert(nickname);
}

void Channel::removeMember(const std::string &nickname)
{
    this->_members.erase(nickname);
}

bool Channel::isMember(const std::string &nickname) const
{
    return this->_members.find(nickname) != this->_members.end();
}

size_t Channel::getMemberCount() const
{
    return this->_members.size();
}

const std::set<std::string> &Channel::getMembers() const
{
    return this->_members;
}

/* ------------------------------------ Operators ----------------------------------- */

void Channel::addOperator(const std::string &nickname)
{
    this->_operators.insert(nickname);
}

void Channel::removeOperator(const std::string &nickname)
{
    this->_operators.erase(nickname);
}

bool Channel::isOperator(const std::string &nickname) const
{
    return this->_operators.find(nickname) != this->_operators.end();
}

size_t Channel::getOperatorCount() const
{
    return this->_operators.size();
}

const std::set<std::string> &Channel::getOperators() const
{
    return this->_operators;
}

/* ------------------------------------ Invited ----------------------------------- */

bool Channel::isInvited(const std::string &nickname) const
{
    return this->_invited.find(nickname) != this->_invited.end();
}

void Channel::addInvited(const std::string &nickname)
{
    this->_invited.insert(nickname);
}

void Channel::removeInvited(const std::string &nickname)
{
    this->_invited.erase(nickname);
}

/* ------------------------------------ Print ----------------------------------- */


// da modificare dopo aver creato i getters
void Channel::printChannelInfo()
{
	std::cout << "Channel: " << getName()
		<< ", Topic: " << getTopic()
		<< ", member count: " << getMemberCount()
		<< ", operator count: " << getOperatorCount()
		<< std::endl;
}