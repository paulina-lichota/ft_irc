#include "Message.hpp"
#include <iostream>

void printMessage(const Message &msg, const std::string &raw) {
	std::cout << "RAW: [" << raw << "]" << std::endl;

	std::cout << "Prefix: " << msg.getPrefix() << std::endl;
	std::cout << "Command: " << msg.getCommand() << std::endl;

	const std::vector<std::string> &params = msg.getParams();
	for (size_t i = 0; i < params.size(); i++) {
		std::cout << "Param[" << i << "]: " << params[i] << std::endl;
	}

	std::cout << "Valid: " << (msg.isValid() ? "yes" : "no") << std::endl;
	std::cout << "Error: " << msg.getError() << std::endl;

	std::cout << "============================\n" << std::endl;
}

int main() {

	// 🔹 BASIC OK
	std::string t1 = "PING";

	// 🔹 trailing senza prefix
	std::string t2 = "PRIVMSG #chan :ciao bello come stai";

	// 🔹 multiple params
	std::string t3 = "COMMAND p1 p2 p3 p4";

	// 🔹 prefix + trailing
	std::string t4 = ":nick!user@host PRIVMSG #chan :hello world";

	// 🔹 spazi multipli strani
	std::string t5 = "   PRIVMSG    #chan     :hello    world   ";

	// 🔹 trailing vuoto
	std::string t6 = "PRIVMSG #chan: ciao";

	// 🔹 solo prefix (ERRORE)
	std::string t7 = ":ciao";

	// 🔹 comando vuoto (ERRORE)
	std::string t8 = "";

	// 🔹 comando + solo spazi dopo
	std::string t9 = "NICK     ";

	// 🔹 params senza trailing ma con :
	std::string t10 = "CMD p1 p2 :trailing with spaces here";

	// 🔹 roba borderline tipo IRC reale
	std::string t11 = ":server.name 372 nick :- Message of the day -";

	// 🔹 doppio ':' nel mezzo (deve prendere tutto dopo il primo)
	std::string t12 = "PRIVMSG #chan :ciao :questo NON è un altro param";

	// 🔹 newline già tolto (simula il tuo buffer)
	std::string t13 = ":nick CMD param1 param2 :final message";

	std::vector<std::string> tests;
	tests.push_back(t1);
	tests.push_back(t2);
	tests.push_back(t3);
	tests.push_back(t4);
	tests.push_back(t5);
	tests.push_back(t6);
	tests.push_back(t7);
	tests.push_back(t8);
	tests.push_back(t9);
	tests.push_back(t10);
	tests.push_back(t11);
	tests.push_back(t12);
	tests.push_back(t13);

	for (size_t i = 0; i < tests.size(); i++) {
		Message msg(tests[i]);
		printMessage(msg, tests[i]);
	}

	return 0;
}
// 111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111112222224
