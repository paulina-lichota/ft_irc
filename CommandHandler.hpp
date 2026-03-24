#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "Message.hpp"

class CommandHandler {
public:
	// void handleJoin(const Message& msg);
	// void handlePrivmsg(const Message& msg);
	// void handleNick(const Message& msg);

	void handle(const Message& msg);

private:
	typedef void (CommandHandler::*HandlerFunc)(const Message&);

	std::map<std::string, HandlerFunc> _handlers;
};

#endif
