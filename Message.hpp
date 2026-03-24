#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

class Message {
private:
	std::string					_prefix;
	std::string					_command;
	std::vector<std::string>	_params;

	bool						_valid;
	std::string					_error;
public:
	Message();
	Message(const std::string &raw);
	Message(const Message& other);
	Message& operator=(const Message& other);
	~Message();

	const std::string&				getPrefix()		const;
	const std::string&				getCommand()	const;
	const std::vector<std::string>&	getParams()		const;
	bool							isValid()		const;
	const std::string&				getError()		const;
};

#endif
