#include "irc.hpp"
#include "server.hpp"

#include <signal.h>

server *s_ptr = nullptr;

void signal_handler(int sig) {
	if (s_ptr != nullptr) {
		s_ptr->stop();
	}
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: ircserv port password" << std::endl;
		return 1;
	}
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	try {
		server s(std::stoi(argv[1]), argv[2]);
		s_ptr = &s;
		s.start();
	} catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
