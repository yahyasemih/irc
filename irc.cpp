#include "irc.hpp"
#include "server.hpp"

int main(int argc, char *argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: ircserv port password" << std::endl;
		return 1;
	}
	try {
		server s(std::stoi(argv[1]), argv[2]);
		s.start();
	} catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
