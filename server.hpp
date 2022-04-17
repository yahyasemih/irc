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

class server {
private:
	static const int BUFFER_SIZE = 100;
	std::string password;
	std::vector<pollfd> pollfds;
	std::unordered_map<int, client> clients;
	std::unordered_map<std::string, int> nick_to_fd;
	int fd;
	sockaddr_in addr;

	void accept_client();
	void receive_from_client(client &c);
	void clear_disconnected_clients(const std::vector<int> &disconnected);
	int handle_command(const std::string &line, client &c);
public:
	server(int port, std::string password);
	~server();

	void start();
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
							std::cout << "command is : " << c.get_command() << std::endl;
							std::cout << "remain stream is : " << c.get_stream().str() << std::endl;
							std::cout << "sending to " << c.get_fd() << std::endl;
							// send(c.get_fd(), ":irc.example.net 001 yahya :Welcome to the Internet Relay Network yahya!~yahya@localhost\r\n"
							// 		":irc.example.net 002 yahya :Your host is irc.example.net, running version ngircd-26.1 (x86_64/apple/darwin18.7.0)\r\n"
							// 		":irc.example.net 003 yahya :This server has been started Mon Mar 21 2022 at 21:25:26 (+01)\r\n"
							// 		":irc.example.net 004 yahya irc.example.net ngircd-26.1 abBcCFiIoqrRswx abehiIklmMnoOPqQrRstvVz\r\n"
							// 		":irc.example.net 005 yahya RFC2812 IRCD=ngIRCd CHARSET=UTF-8 CASEMAPPING=ascii PREFIX=(qaohv)~&@%+ CHANTYPES=#&+ CHANMODES=beI,k,l,imMnOPQRstVz CHANLIMIT=#&+:10 :are supported on this server\r\n"
							// 		":irc.example.net 005 yahya CHANNELLEN=50 NICKLEN=9 TOPICLEN=490 AWAYLEN=127 KICKLEN=400 MODES=5 MAXLIST=beI:50 EXCEPTS=e INVEX=I PENALTY FNC :are supported on this server\r\n"
							// 		":irc.example.net 251 yahya :There are 1 users and 0 services on 1 servers\r\n"
							// 		":irc.example.net 254 yahya 1 :channels formed\r\n"
							// 		":irc.example.net 255 yahya :I have 1 users, 0 services and 0 servers\r\n"
							// 		":irc.example.net 265 yahya 1 1 :Current local users: 1, Max: 1\r\n"
							// 		":irc.example.net 266 yahya 1 1 :Current global users: 1, Max: 1\r\n"
							// 		":irc.example.net 250 yahya :Highest connection count: 1 (2 connections received)\r\n"
							// 		":irc.example.net 422 yahya :MOTD file is missing", 1208, 0);
							//c.get_stream().clear();
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

int server::handle_command(const std::string &line, client &c) {
	std::stringstream stream(line);
	std::string cmd, prefix;
	std::vector<std::string> args;
	stream >> cmd;
	if (cmd[0] == ':')
		stream >> cmd; // ignoring prefix
	if (cmd == "PASS") {
		if (c.connection_already_registered())
			return 462;
		std::string p, r;
		stream >> p >> r;
		if (p[0] == ':')
			p = p.substr(1);
		if (p.empty() || !r.empty())
			return 461;
		if (p != password)
			return 464;
		c.set_pass(p);
	} else if (cmd == "NICK") {
		std::string nname, r;
		stream >> nname >> r;
		if (nname[0] == ':')
			nname = nname.substr(1);
		if (nname.empty() || !r.empty());
			return 461;
		if (!c.get_username().empty()) {
			if (c.get_pass() != password)
				return 464;
		} else {
			if (!c.get_nickname().empty()) {
				std::cout << "User "<< c.get_username() << " changed nick " << c.get_nickname() << " -> " << nname
						<< std::endl;
			}
			c.set_nickname(nname);
		}
	} else if (cmd == "USER") {
	}
}

#endif
