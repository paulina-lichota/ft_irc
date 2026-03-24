#include "CommandHandler.hpp"

void CommandHandler::handle(const Message& msg) {
	std::map<std::string, HandlerFunc>::iterator it;

	it = _handlers.find(msg.getCommand());
	if (it != _handlers.end()) {
		HandlerFunc func = it->second;
		(this->*func)(msg);
	} else {
		// comando sconosciuto
	}
}
