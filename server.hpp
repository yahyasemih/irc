#ifndef SERVER_HPP
#define SERVER_HPP

#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

#include "client.hpp"
#include "command_parser.hpp"

class server {
private:
	static const int BUFFER_SIZE = 100;
	typedef std::unordered_map<std::string, command_type> command_map;
	typedef int (server::*command_function)(const command_parser &cmd, client &c);
	static const command_map str_to_cmd;
	static const command_function command_functions[5];
	std::string password;
	std::vector<pollfd> pollfds;
	std::unordered_map<int, client> clients;
	std::unordered_map<std::string, int> nick_to_fd;
	int fd;
	sockaddr_in addr;

	void accept_client();
	void receive_from_client(client &c);
	void clear_disconnected_clients(const std::vector<int> &disconnected);
	int handle_command(const std::string &line, client &c, std::string &reply);

	int nick_cmd(const command_parser &cmd, client &c);
	int user_cmd(const command_parser &cmd, client &c);
	int pass_cmd(const command_parser &cmd, client &c);
	int die_cmd(const command_parser &cmd, client &c);

	static command_map init_map();
public:
	server(int port, std::string password);
	~server();

	void start();
};

server::command_map server::init_map() {
	command_map map;
	map.insert(std::make_pair("nick", NICK));
	map.insert(std::make_pair("user", USER));
	map.insert(std::make_pair("pass", PASS));
	map.insert(std::make_pair("die", DIE));
	return map;
}

const server::command_map server::str_to_cmd = init_map();

const server::command_function server::command_functions[5] = {
	&server::nick_cmd,
	&server::user_cmd,
	&server::pass_cmd,
	&server::die_cmd,
	nullptr
};

server::server(int port, std::string password) {
	this->password = password;
	protoent *protocol;
	protocol = getprotobyname("tcp");
	if (protocol == NULL) {
		throw std::runtime_error("Error while getting protocol TCP");
	}
	fd = socket(AF_INET, SOCK_STREAM, protocol->p_proto);
	if (fd < 0) {
		throw std::runtime_error("Socket creation failed");
	}
	int options = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &options, sizeof(options))) {
		perror("Socket options : ");
		throw std::runtime_error("Setting socket options failed : ");
	}
	fcntl(fd, F_SETFL, O_NONBLOCK);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Binding failed : ");
		throw std::runtime_error("Binding failed");
	}
	if (listen(fd, 3) < 0) {
		throw std::runtime_error("Listening failed");
	}
	pollfd pf;
	pf.fd = fd;
	pf.events = POLLSTANDARD;
	pollfds.push_back(pf);
	std::cout << "Server successfully initialised on port " << port << std::endl;
}

server::~server() {
}

void server::start() {
	while (true) {
		int r = poll(pollfds.data(), pollfds.size(), 0);
		if (r > 0) {
			if (pollfds[0].revents & POLLRDNORM) {
				accept_client();
			}
			std::vector<int> disconnected;
			for (int i = 1; i < pollfds.size(); ++i) {
				pollfd &pf  = pollfds[i];
				if (pf.revents & POLLIN) {
					std::unordered_map<int, client>::iterator it = clients.find(pf.fd);
					if (it != clients.end()) {
						client &c = it->second;
						receive_from_client(c);
						while (c.has_command()) {
							std::string reply;
							int reply_code = handle_command(c.get_command(), c, reply);
							if (reply_code == 400) {
								std::string final_reply = "ERROR :" + reply + "\r\n";
								send(c.get_fd(), final_reply.c_str(), final_reply.size(), 0);
								c.get_stream().clear();
								close(c.get_fd());
								disconnected.push_back(i);
								clients.erase(pf.fd);
								std::cout << "client disconnected" << std::endl;
							}
						}
					}
				}
				if (pf.revents & POLLHUP) {
					disconnected.push_back(i);
					clients.erase(pf.fd);
					std::cout << "client disconnected" << std::endl;
				}
			}
			if (!disconnected.empty()) {
				clear_disconnected_clients(disconnected);
			}
		}
	}
}

void server::accept_client() {
	int addrlen = sizeof(addr);
	int new_socket = accept(fd, (sockaddr *)&addr, (socklen_t *)&addrlen);

	if (new_socket < 0) {
		perror("Could not accept connection : ");
	} else {
		fcntl(new_socket, F_SETFL, O_NONBLOCK);
		pollfd pf;
		pf.fd = new_socket;
		pf.events = POLLIN;
		pollfds.push_back(pf);
		clients.insert(std::make_pair(new_socket, client(new_socket)));
	}
}

void server::receive_from_client(client &c) {
	char buff[BUFFER_SIZE + 1];
	int ret = recv(c.get_fd(), buff, BUFFER_SIZE, 0);
	buff[ret] = '\0';
	c.receive(buff);
}

void server::clear_disconnected_clients(const std::vector<int> &disconnected) {
	for (int i = 0; i < disconnected.size(); ++i) {
		pollfds.erase(pollfds.begin() + disconnected[i]);
	}
}

int server::handle_command(const std::string &line, client &c, std::string &reply) {
	if (line.size() > 510) {
		reply = ":Request too long";
		return 400; // ERROR
	}
	command_parser cmd_parser(line);
	if (c.is_connected() && !cmd_parser.get_prefix().empty() && cmd_parser.get_prefix().substr(1) != c.get_nickname()) {
		reply = ":Invalid prefix \"" + cmd_parser.get_prefix().substr(1) + "\"";
		return 400; // ERROR
	}
	command_map::const_iterator it = str_to_cmd.find(cmd_parser.get_cmd());
	command_type cmd_type = it == str_to_cmd.cend() ? INVALID_CMD : it->second;
	if (cmd_type == INVALID_CMD) {
		reply = cmd_parser.get_cmd() + " :Unknown command";
		return 421; // :Unknown command
	} else {
		return (this->*(command_functions[cmd_type]))(cmd_parser, c);
	}
}

int server::pass_cmd(const command_parser &cmd, client &c) {
	if (c.connection_already_registered()) {
		return 462;
	} else if (cmd.get_args().size() != 1) {
		return 461;
	} else {
		c.set_pass(cmd.get_args().at(0));
		return 0;
	}
}

int server::nick_cmd(const command_parser &cmd, client &c) {
	if (cmd.get_args().size() != 1) {
		return 461;
	}
	std::string nickname = cmd.get_args().at(0);
	if (!c.get_username().empty()) {
		if (c.get_pass() != password) {
			return 464;
		} else {
			c.set_nickname(nickname);
			c.set_connected(true);
			return 0;
		}
	} else {
		c.set_nickname(nickname);
		c.set_connected(true);
		return 0;
	}
}

int server::user_cmd(const command_parser &cmd, client &c) {
	// TODO: fix chaining/checking of checking errors (cnx already/not registered cases)
	if (c.connection_not_registered()) {
		return 451;
	} else if (c.connection_already_registered()) {
		return 462;
	} else if (cmd.get_args().size() != 1) {
		return 461;
	}
	std::string username = cmd.get_args().at(0);
	if (!c.get_nickname().empty()) {
		if (c.get_pass() != password) {
			return 464;
		} else {
			c.set_username(username);
			c.set_connected(true);
			return 0;
		}
	} else {
		c.set_username(username);
		c.set_connected(true);
		return 0;
	}
}

int server::die_cmd(const command_parser &cmd, client &c) {
	if (c.connection_not_registered()) {
		return 451;
	} else if (cmd.get_args().size() > 1) {
		return 461;
	} else if (!c.is_oper()) {
		return 481; // Permission denied
	} else {
		// TODO: broadcast notice + args[0] if args[0] exists
		// TODO: broadcast notice of connection stats
		// TODO: set reply as ":Server going down"
		return 400; // ERROR
	}
	return 0;
}
#endif
