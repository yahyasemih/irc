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
#include "channel.hpp"
#include "command_parser.hpp"
#include "server_config.hpp"

class server {
private:
	typedef std::unordered_map<std::string, command_type> command_map;
	typedef std::unordered_map<std::string, channel> channel_map;
	typedef std::unordered_map<int, client> client_map;
	typedef int (server::*command_function)(const command_parser &, client &, std::string &);

	static const int BUFFER_SIZE = 100;
	static const command_map str_to_cmd;
	static const command_function command_functions[INVALID_CMD];

	std::string password;
	std::vector<pollfd> pollfds;
	std::unordered_map<int, client> clients;
	channel_map channels;
	std::unordered_map<std::string, int> nick_to_fd;
	int fd;
	sockaddr_in addr;
	server_config config;

	void accept_client();
	void receive_from_client(client &c);
	void clear_disconnected_clients(const std::vector<int> &disconnected);
	int handle_command(const std::string &line, client &c, std::string &reply);

	int nick_cmd(const command_parser &cmd, client &c, std::string &reply);
	int user_cmd(const command_parser &cmd, client &c, std::string &reply);
	int pass_cmd(const command_parser &cmd, client &c, std::string &reply);
	int oper_cmd(const command_parser &cmd, client &c, std::string &reply);
	int die_cmd(const command_parser &cmd, client &c, std::string &reply);
	int privmsg_cmd(const command_parser &cmd, client &c, std::string &reply);
	int join_cmd(const command_parser &cmd, client &c, std::string &reply);
	int quit_cmd(const command_parser &cmd, client &c, std::string &reply);
	int part_cmd(const command_parser &cmd, client &c, std::string &reply);
	int away_cmd(const command_parser &cmd, client &c, std::string &reply);
	int topic_cmd(const command_parser &cmd, client &c, std::string &reply);
	int mode_cmd(const command_parser &cmd, client &c, std::string &reply);
	int users_cmd(const command_parser &cmd, client &c, std::string &reply);
	int stats_cmd(const command_parser &cmd, client &c, std::string &reply);
	int info_cmd(const command_parser &cmd, client &c, std::string &reply);
	int invite_cmd(const command_parser &cmd, client &c, std::string &reply);
	int kick_cmd(const command_parser &cmd, client &c, std::string &reply);
	int names_cmd(const command_parser &cmd, client &c, std::string &reply);
	int list_cmd(const command_parser &cmd, client &c, std::string &reply);
	int notice_cmd(const command_parser &cmd, client &c, std::string &reply);

	static command_map init_map();
public:
	server(int port, std::string password, std::string config_file = "");
	~server();

	void start();
};

server::command_map server::init_map() {
	command_map map;

	map.insert(std::make_pair("nick", NICK));
	map.insert(std::make_pair("user", USER));
	map.insert(std::make_pair("pass", PASS));
	map.insert(std::make_pair("oper", OPER));
	map.insert(std::make_pair("die", DIE));
	map.insert(std::make_pair("privmsg", PRIVMSG));
	map.insert(std::make_pair("join", JOIN));
	map.insert(std::make_pair("quit", QUIT));
	map.insert(std::make_pair("part", PART));
	map.insert(std::make_pair("away", AWAY));
	map.insert(std::make_pair("topic", TOPIC));
	map.insert(std::make_pair("mode", MODE));
	map.insert(std::make_pair("users", USERS));
	map.insert(std::make_pair("stats", STATS));
	map.insert(std::make_pair("info", INFO));
	map.insert(std::make_pair("invite", INVITE));
	map.insert(std::make_pair("kick", KICK));
	map.insert(std::make_pair("names", NAMES));
	map.insert(std::make_pair("list", LIST));
	map.insert(std::make_pair("notice", NOTICE));

	return map;
}

const server::command_map server::str_to_cmd = init_map();

const server::command_function server::command_functions[INVALID_CMD] = {
	&server::nick_cmd,
	&server::user_cmd,
	&server::pass_cmd,
	&server::oper_cmd,
	&server::die_cmd,
	&server::privmsg_cmd,
	&server::join_cmd,
	&server::quit_cmd,
	&server::part_cmd,
	&server::away_cmd,
	&server::topic_cmd,
	&server::mode_cmd,
	&server::users_cmd,
	&server::stats_cmd,
	&server::info_cmd,
	&server::invite_cmd,
	&server::kick_cmd,
	&server::names_cmd,
	&server::list_cmd,
	&server::notice_cmd
};

server::server(int port, std::string password, std::string config_file) {
	this->config = config_file.empty() ? server_config() : server_config(config_file);
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
					client_map::iterator it = clients.find(pf.fd);
					if (it != clients.end()) {
						client &c = it->second;
						receive_from_client(c);
						while (c.has_command()) {
							std::string reply;
							int reply_code = handle_command(c.get_command(), c, reply);
							if (reply_code == -1) {
								std::string final_reply = "ERROR " + reply + "\r\n";
								send(c.get_fd(), final_reply.c_str(), final_reply.size(), 0);
								c.get_stream().clear();
								close(c.get_fd());
								disconnected.push_back(i);
								clients.erase(pf.fd);
								std::cout << "client disconnected" << std::endl;
							} else if (reply_code >= 400 && reply_code <= 599) {
								std::string final_reply = config.get_server_name() + " " + std::to_string(reply_code);
								final_reply += " " + c.get_nickname() + " " + reply + "\r\n";
								send(c.get_fd(), final_reply.c_str(), final_reply.size(), 0);
							} else {
								std::cout << "code: " << reply_code << ", reply: '" << reply << "'" << std::endl;
								send(c.get_fd(), reply.c_str(), reply.size(), 0);
							}
						}
					}
				}
				if (pf.revents & POLLHUP) {
					if (std::find(disconnected.begin(), disconnected.end(), i) == disconnected.end()) {
						disconnected.push_back(i);
						clients.erase(pf.fd);
						std::cout << "client disconnected" << std::endl;
					}
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
		return -1; // FATAL ERROR
	}
	command_parser cmd_parser(line);
	if (c.is_connected() && !cmd_parser.get_prefix().empty() && cmd_parser.get_prefix().substr(1) != c.get_nickname()) {
		reply = ":Invalid prefix \"" + cmd_parser.get_prefix().substr(1) + "\"";
		return 400; // ERROR
	}
	if (cmd_parser.get_cmd().empty()) {
		return 0;
	}
	command_map::const_iterator it = str_to_cmd.find(cmd_parser.get_cmd_lowercase());
	command_type cmd_type = it == str_to_cmd.cend() ? INVALID_CMD : it->second;
	if (cmd_type == INVALID_CMD) {
		reply = cmd_parser.get_cmd() + " :Unknown command";
		return 421; // :Unknown command
	} else {
		return (this->*(command_functions[cmd_type]))(cmd_parser, c, reply);
	}
}

int server::pass_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (c.connection_already_registered()) {
		return 462;
	} else if (cmd.get_args().size() != 1) {
		reply = cmd.get_cmd() + " :Syntax error\r\n";
		return 461;
	} else {
		c.set_pass(cmd.get_args().at(0));
		return 0;
	}
}

int server::nick_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (cmd.get_args().size() != 1) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	}
	std::string nickname = cmd.get_args().at(0);
	if (!c.get_username().empty()) {
		if (c.get_pass() != password) {
			reply = ":Password incorrect";
			return -1;
		} else {
			if (c.get_nickname() != nickname) {
				if (c.is_connected() && !c.get_nickname().empty()) {
					reply = ":" + c.get_nickname() + " NICK :" + nickname;
				}
				nick_to_fd.erase(c.get_nickname());
				c.set_nickname(nickname);
				nick_to_fd.insert(std::make_pair(nickname, c.get_fd()));
			}
			c.set_connected(true);
			return 0;
		}
	}
	if (c.get_nickname() != nickname) {
		if (c.is_connected() && !c.get_nickname().empty()) {
			reply = ":" + c.get_nickname() + " NICK :" + nickname;
		}
		nick_to_fd.erase(c.get_nickname());
		c.set_nickname(nickname);
		nick_to_fd.insert(std::make_pair(nickname, c.get_fd()));
	}
	return 0;
}

int server::user_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO: fix chaining/checking of checking errors (cnx already/not registered cases)
	if (c.connection_not_registered()) {
		return 451;
	} else if (c.connection_already_registered()) {
		return 462;
	} else if (cmd.get_args().size() != 4) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	}
	std::string username = cmd.get_args().at(0);
	std::string mode = cmd.get_args().at(1);
	std::string realname = cmd.get_args().at(3);
	if (!c.get_nickname().empty()) {
		if (c.get_pass() != password) {
			reply = ":Password incorrect";
			return -1;
		} else {
			c.set_username(username);
			c.set_realname(realname);
			c.set_connected(true);
			return 0;
		}
	} else {
		c.set_username(username);
		c.set_realname(realname);
		return 0;
	}
}

int server::die_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (c.connection_not_registered()) {
		return 451;
	} else if (cmd.get_args().size() > 1) {
		reply = cmd.get_cmd() + " :Syntax error";
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

int server::oper_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (c.connection_not_registered()) {
		reply = ":Connection not registered";
		return 451;
	} else if (cmd.get_args().size() != 2) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	} else {
		const std::string &name = cmd.get_args().at(0);
		const std::string &pass = cmd.get_args().at(1);
		server_config::operator_map::const_iterator it = config.get_operators().find(name);
		if (it == config.get_operators().cend() || it->second != pass) {
			reply = ":Invalid password";
			return 464;
		}
		reply = "";
		if (c.is_oper() == false) {
			reply += ":+o\r\n";
			c.set_oper(true);
		}
		reply += ":You are now an IRC Operator\r\n";
		return 381;
	}
}

int server::privmsg_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (cmd.get_args().empty()) {
		reply = ":No recipient given (" + cmd.get_cmd() + ")";
		return 411;
	} else if (cmd.get_args().size() == 1) {
		reply = ":No text to send";
		return 412;
	} else if (cmd.get_args().size() > 2) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	} else {
		const std::string &receiver = cmd.get_args().at(0);
		const std::string &text = cmd.get_args().at(1);
		std::unordered_map<std::string, int>::iterator nicks_it = nick_to_fd.find(receiver);
		channel_map::iterator channels_it = channels.find(receiver);
		if (nicks_it == nick_to_fd.cend() && channels_it == channels.end()) {
			reply = receiver + " :No such nick or channel name";
			return 401;
		} else {
			std::string msg = ":" + c.get_nickname() + " PRIVMSG " + receiver + " :" + text + "\r\n";
			if (channels_it != channels.end()) {
				channels_it->second.send_message(msg, &c);
			} else {
				send(nicks_it->second, msg.c_str(), msg.size(), 0);
			}
			return 0;
		}
	}
}

int server::join_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (cmd.get_args().empty() || cmd.get_args().size() > 2) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	}
	std::string chnl = cmd.get_args().at(0);
	if (channels.find(chnl) == channels.end()) {
		channel ch = cmd.get_args().size() == 1 ? channel() : channel(cmd.get_args().at(1));
		ch.add_client(&c);
		channels.insert(std::make_pair(chnl, ch));
		std::string msg = ":" + c.get_nickname() + " JOIN :" + chnl + "\r\n";
		msg += ":irc.1337.ma 353 " + c.get_nickname() + " = " + chnl + ":@" + c.get_nickname() + "\r\n"; // TODO: generate correct messaeg based on users in channel and correct privileges
		msg += ":irc.1337.ma 366 " + c.get_nickname() + " " + chnl + " :End of NAMES list\r\n";
		ch.send_message(msg, nullptr);
	} else {
		if (!channels[chnl].is_in_channel(&c)) {
			channels[chnl].add_client(&c);
			std::string msg = ":" + c.get_nickname() + " JOIN :" + chnl + "\r\n";
			msg += ":irc.1337.ma 353 " + c.get_nickname() + " = " + chnl + ":@" + c.get_nickname() + "\r\n"; // TODO: generate correct messaeg based on users in channel and correct privileges
			msg += ":irc.1337.ma 366 " + c.get_nickname() + " " + chnl + " :End of NAMES list\r\n";
			channels[chnl].send_message(msg, nullptr);
		}
	}

	return 0;
}

int server::quit_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (cmd.get_args().size() > 1) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	}
	reply = cmd.get_args().empty() ? ":Closing connection" : ":" + cmd.get_args().at(0);
	for (channel_map::iterator it = channels.begin(); it != channels.end(); ++it) {
		if (it->second.is_in_channel(&c)) {
			std::string msg = ":" + c.get_nickname() + " QUIT :";
			msg += cmd.get_args().empty() ? c.get_nickname() : cmd.get_args().at(0);
			it->second.send_message(msg, &c);
			it->second.remove_client(&c);
		}
	}
	return -1; // FATAL ERROR
}

int server::part_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (cmd.get_args().empty() || cmd.get_args().size() > 2) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	}
	const std::string &channel_name = cmd.get_args().at(0);
	channel_map::iterator it = channels.find(channel_name);
	if (it == channels.end()) {
		reply = channel_name + " :No such channel";
		return 403;
	} else if (!it->second.is_in_channel(&c)) {
		reply = channel_name + " :You are not on that channel";
		return 442;
	} else {
		std::string msg = cmd.get_args().size() == 1 ? "" : cmd.get_args().at(1);
		msg = ":" + c.get_nickname() + " PART " + channel_name + ":" + msg + "\r\n";
		it->second.send_message(msg, nullptr);
		it->second.remove_client(&c);
		return 0; // No reply to send, already sent by channel
	}
}

int server::away_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	return 0;
}

int server::topic_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	return 0;
}

int server::mode_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	return 0;
}

int server::users_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	return 0;
}

int server::stats_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	return 0;
}

int server::info_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	return 0;
}

int server::invite_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	return 0;
}

int server::kick_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	return 0;
}

int server::names_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	return 0;
}

int server::list_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	return 0;
}

int server::notice_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	return 0;
}

#endif
