#include "server.hpp"

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
	map.insert(std::make_pair("ping", PING));
	map.insert(std::make_pair("pong", PING));
	map.insert(std::make_pair("who", WHO));
	map.insert(std::make_pair("whois", WHO));

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
	&server::notice_cmd,
	&server::ping_cmd,
	&server::pong_cmd,
	&server::who_cmd,
	&server::whois_cmd
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
	is_running = true;
	while (is_running) {
		int r = poll(pollfds.data(), pollfds.size(), 0);
		if (r > 0) {
			if (pollfds[0].revents & POLLRDNORM) {
				accept_client();
			}
			std::vector<int> disconnected;
			for (size_t i = 1; i < pollfds.size(); ++i) {
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
								disconnected.push_back(i);
								join_cmd(command_parser("JOIN 0"), c, reply);
								nick_to_fd.erase(c.get_nickname());
								clients.erase(pf.fd);
								std::cout << "client disconnected" << std::endl;
							} else if (reply_code == 400) {
								std::string final_reply = "ERROR " + reply + "\r\n";
								send(c.get_fd(), final_reply.c_str(), final_reply.size(), 0);
							} else if (reply_code > 0 && reply_code <= 599) {
								std::string final_reply = ":" + config.get_server_name() + " " + std::to_string(reply_code);
								std::string nickname = c.get_nickname().empty() ? "*" : c.get_nickname();
								final_reply += " " + nickname + " " + reply + "\r\n";
								send(c.get_fd(), final_reply.c_str(), final_reply.size(), 0);
							} else if (reply_code != 0) {
								send(c.get_fd(), reply.c_str(), reply.size(), 0);
							}
						}
					}
				}
				if (pf.revents & POLLHUP) {
					if (std::find(disconnected.begin(), disconnected.end(), i) == disconnected.end()) {
						disconnected.push_back(i);
						std::string reply;
						join_cmd(command_parser("JOIN 0"), clients.find(pf.fd)->second, reply);
						nick_to_fd.erase(clients.find(pf.fd)->second.get_nickname());
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

void server::stop() {
	is_running = false;
	std::cout << "\rClosing server" << std::endl;
	for (size_t i = 0; i < pollfds.size(); ++i){
		std::string msg = "ERROR :Server going down\r\n";
		send(pollfds[i].fd, msg.c_str(), msg.size(), 0);
		// TODO: if pd is in clients map then send notice (maybe)
	}
}

void server::welcome_client(const client &c) const {
	std::string msg = ":" + config.get_server_name() + " 001 " + c.get_nickname() + " :Welcome to the Internet Relay Network " + c.to_string() + "\r\n"
			":" + config.get_server_name() + " 002 " + c.get_nickname() + " :Your host is " + config.get_server_name() + ", running version ngircd-26.1 (x86_64/apple/darwin18.7.0)\r\n"
			":" + config.get_server_name() + " 003 " + c.get_nickname() + " :This server has been started Mon Mar 21 2022 at 21:25:26 (+01)\r\n"
			":" + config.get_server_name() + " 004 " + c.get_nickname() + " " + config.get_server_name() + " ngircd-26.1 abBcCFiIoqrRswx abehiIklmMnoOPqQrRstvVz\r\n"
			":" + config.get_server_name() + " 005 " + c.get_nickname() + " RFC2812 IRCD=ngIRCd CHARSET=UTF-8 CASEMAPPING=ascii PREFIX=(qaohv)~&@%+ CHANTYPES=#&+ CHANMODES=beI,k,l,imMnOPQRstVz CHANLIMIT=#&+:10 :are supported on this server\r\n"
			":" + config.get_server_name() + " 005 " + c.get_nickname() + " CHANNELLEN=50 NICKLEN=9 TOPICLEN=490 AWAYLEN=127 KICKLEN=400 MODES=5 MAXLIST=beI:50 EXCEPTS=e INVEX=I PENALTY FNC :are supported on this server\r\n"
			":" + config.get_server_name() + " 251 " + c.get_nickname() + " :There are 1 users and 0 services on 1 servers\r\n"
			":" + config.get_server_name() + " 254 " + c.get_nickname() + " 1 :channels formed\r\n"
			":" + config.get_server_name() + " 255 " + c.get_nickname() + " :I have 1 users, 0 services and 0 servers\r\n"
			":" + config.get_server_name() + " 265 " + c.get_nickname() + " 1 1 :Current local users: 1, Max: 1\r\n"
			":" + config.get_server_name() + " 266 " + c.get_nickname() + " 1 1 :Current global users: 1, Max: 1\r\n"
			":" + config.get_server_name() + " 250 " + c.get_nickname() + " :Highest connection count: 1 (2 connections received)\r\n"
			":" + config.get_server_name() + " 422 " + c.get_nickname() + " :MOTD file is missing\r\n";
		send(c.get_fd(), msg.c_str(), msg.size(), 0);
}

void server::accept_client() {
	int addrlen = sizeof(addr);
	int new_socket = accept(fd, (sockaddr *)&addr, (socklen_t *)&addrlen);

	if (new_socket < 0) {
		perror("Could not accept connection : ");
	} else {
		fcntl(new_socket, F_SETFL, O_NONBLOCK);
		pollfd pf;
		std::string host = inet_ntoa(addr.sin_addr); // TODO: reverse dns
		pf.fd = new_socket;
		pf.events = POLLIN;
		pollfds.push_back(pf);
		clients.insert(std::make_pair(new_socket, client(new_socket, host)));
	}
}

void server::receive_from_client(client &c) {
	char buff[BUFFER_SIZE + 1];
	int ret = recv(c.get_fd(), buff, BUFFER_SIZE, 0);
	buff[ret] = '\0';
	c.receive(buff);
}

void server::clear_disconnected_clients(const std::vector<int> &disconnected) {
	for (size_t i = 0; i < disconnected.size(); ++i) {
		pollfds.erase(pollfds.begin() + disconnected[i]);
	}
}

std::string server::get_clients_without_channel() const {
	std::string result;
	if (channels.empty()) {
		for (client_map::const_iterator client_it = clients.begin(); client_it != clients.end(); ++client_it) {
			result += (result.empty() ? "" : " ") + client_it->second.get_nickname();
		}
	} else {
		for (client_map::const_iterator client_it = clients.begin(); client_it != clients.end(); ++client_it) {
			for (channel_map::const_iterator channel_it = channels.cbegin(); channel_it != channels.cend(); ++channel_it) {
				if (!channel_it->second.is_in_channel(&(const_cast<client &>(client_it->second)))) {
					result += (result.empty() ? "" : " ") + client_it->second.get_nickname();
				}
			}
		}
	}
	return result;
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
		return 421;
	} else {
		return (this->*(command_functions[cmd_type]))(cmd_parser, c, reply);
	}
}

int server::pass_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (c.connection_already_registered()) {
		reply = ":Unauthorized command (already registered)";
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
	if (nickname.empty() || nickname.size() > 9
			|| (strchr("[]\\`_^{|}", nickname[0]) == nullptr && !isalpha(nickname[0]))) {
		reply = nickname + " :Erroneous nickname";
		return 432;
	} else {
		for (size_t i = 1; i < nickname.size(); ++i) {
			if (strchr("[]\\`_^{|}-", nickname[i]) == nullptr && !isalnum(nickname[i])) {
				reply = nickname + " :Erroneous nickname";
				return 432;
			}
		}
	}
	if (!c.get_username().empty()) {
		if (c.get_pass() != password) {
			reply = ":Password incorrect";
			return -1;
		} else {
			if (c.get_nickname() != nickname) {
				if (nick_to_fd.find(nickname) != nick_to_fd.end()) {
					reply = nickname + " :Nickname already in use";
					return 433;
				}
				if (c.is_connected() && !c.get_nickname().empty()) {
					reply = ":" + c.to_string() + " NICK :" + nickname + "\r\n";
					send(c.get_fd(), reply.c_str(), reply.size(), 0);
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
		if (nick_to_fd.find(nickname) != nick_to_fd.end()) {
			reply = nickname + " :Nickname already in use";
			return 433;
		}
		if (c.is_connected() && !c.get_nickname().empty()) {
			reply = ":" + c.to_string() + " NICK :" + nickname + "\r\n";
			send(c.get_fd(), reply.c_str(), reply.size(), 0);
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
		reply = ":You have not registered";
		return 451;
	} else if (c.connection_already_registered()) {
		reply = ":Unauthorized command (already registered)";
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
		reply = ":You have not registered";
		return 451;
	} else if (!cmd.get_args().empty()) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	} else if (!c.is_oper()) {
		reply = ":Permission Denied";
		return 481;
	} else {
		stop();
	}
	return 0;
}

int server::oper_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (c.connection_not_registered()) {
		reply = ":You have not registered";
		return 451;
	} else if (cmd.get_args().size() != 2) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	} else {
		const std::string &name = cmd.get_args().at(0);
		const std::string &pass = cmd.get_args().at(1);
		server_config::operator_map::const_iterator it = config.get_operators().find(name);
		if (it == config.get_operators().cend() || it->second != pass) {
			reply = ":Password incorrect";
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
			std::string msg = ":" + c.to_string() + " PRIVMSG " + receiver + " :" + text + "\r\n";
			if (channels_it != channels.end()) {
				if (channels_it->second.is_in_channel(&c)) {
					channels_it->second.send_message(msg, &c);
				} else {
					reply = channels_it->first + " :You are not on that channel";
					return 442;
				}
			} else {
				client &c2 = clients.find(nicks_it->second)->second;
				send(c2.get_fd(), msg.c_str(), msg.size(), 0);
				if (c2.is_away()) {
					reply = receiver + " :" + c2.get_away_msg();
					return 301;
				}
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
	if (cmd.get_args().size() == 1 && cmd.get_args().at(0) == "0") {
		for (channel_map::iterator it = channels.begin(); it != channels.end();) {
			if (it->second.is_in_channel(&c)) {
				std::string msg = ":" + c.to_string() + " PART " + it->first + " :" + c.get_nickname() + "\r\n";
				it->second.send_message(msg, nullptr);
				it->second.remove_client(&c);
			}
			if (it->second.empty()) {
				channel_map::iterator to_erase = it++;
				channels.erase(to_erase);
			} else {
				it++;
			}
		}
		return 0; // No reply to send, already sent by channel
	}
	std::string chnl = cmd.get_args().at(0);
	if (strchr("#+&", chnl[0]) == nullptr || chnl.find(":") != std::string::npos) {
		reply = chnl + " :No such channel";
		return 403;
	} else if (channels.find(chnl) == channels.end()) {
		channel ch = cmd.get_args().size() == 1 ? channel() : channel(cmd.get_args().at(1));
		ch.add_client(&c);
		channels.insert(std::make_pair(chnl, ch));
		std::string msg;
		msg = ":" + c.to_string() + " JOIN :" + chnl + "\r\n";
		send(c.get_fd(), msg.c_str(), msg.size(), 0);
		names_cmd(command_parser("NAMES " + chnl), c, msg);
	} else {
		if (!channels[chnl].is_in_channel(&c)) {
			channels[chnl].add_client(&c);
			std::string msg = ":" + c.to_string() + " JOIN :" + chnl + "\r\n";
			channels[chnl].send_message(msg, nullptr);
			names_cmd(command_parser("NAMES " + chnl), c, msg);
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
	for (channel_map::iterator it = channels.begin(); it != channels.end();) {
		if (it->second.is_in_channel(&c)) {
			std::string msg = ":" + c.to_string() + " QUIT :";
			msg += cmd.get_args().empty() ? c.get_nickname() : cmd.get_args().at(0) + "\r\n";
			// TODO: send also stats notice
			it->second.send_message(msg, &c);
			it->second.remove_client(&c);
			if (it->second.empty()) {
				channel_map::iterator to_erase = it++;
				channels.erase(to_erase);
			} else {
				it++;
			}
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
		msg = ":" + c.to_string() + " PART " + channel_name + " :" + msg + "\r\n";
		it->second.send_message(msg, nullptr);
		it->second.remove_client(&c);
		if (it->second.empty()) {
			channels.erase(it);
		}
		return 0; // No reply to send, already sent by channel
	}
}

int server::away_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (cmd.get_args().size() > 1) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	} else if (cmd.get_args().empty()) {
		c.set_away_msg("");
		reply = ":You are no longer marked as being away";
		return 305;
	} else {
		const std::string msg = cmd.get_args().at(0);
		c.set_away_msg(msg);
		reply = ":You have been marked as being away";
		return 306;
	}
	return 0;
}

int server::topic_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	// using this hack to mute flags IT MUST BE REMOVED AFTER Implenting the function !!!!
	(void)cmd;
	(void)c;
	(void)reply;
	return 0;
}

int server::mode_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO: add channel modes, refactor this function into two parts: user_mode_cmd and channel_mode_cmd
	if (cmd.get_args().empty() || cmd.get_args().size() > 2) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	}
	const std::string &nickname = cmd.get_args().at(0);
	if (nick_to_fd.find(nickname) == nick_to_fd.end() && channels.find(nickname) == channels.end()) {
		reply = nickname + " :No such nick or channel name";
		return 401;
	}
	if (nickname != c.get_nickname()) {
		reply = ":Can't set/get mode for other users";
		return 502;
	}
	if (cmd.get_args().size() == 1) {
		reply = c.get_mode();
		return 221;
	} else {
		const std::string &mode = cmd.get_args().at(1);
		std::string msg;
		std::string result_mode;
		if (mode[0] == '-' || mode[0] == '+') {
			for (size_t i = 1; i < mode.size(); ++i) {
				if (strchr("oO", mode[i]) != nullptr) {
					if (c.is_oper() || c.has_mode(mode[i])) {
						if (mode[0] == '-') {
							if (c.has_mode(mode[i])) {
								result_mode += mode[i];
								c.remove_mode(mode[i]);
							}
						} else {
							if (!c.has_mode(mode[i])) {
								result_mode += mode[i];
								c.add_mode(mode[i]);
							} else {
								msg += ":" + config.get_server_name() + " 481 " + nickname + " :Permission denied\r\n";
							}
						}
					} else {
						if (mode[0] == '+') {
							msg += ":" + config.get_server_name() + " 481 " + nickname + " :Permission denied\r\n";
						}
					}
				} else if (strchr("aiwrs", mode[i]) != nullptr) {
					if (mode[0] == '-') {
						if (c.has_mode(mode[i])) {
							result_mode += mode[i];
							c.remove_mode(mode[i]);
						}
					} else {
						if (!c.has_mode(mode[i])) {
							result_mode += mode[i];
							c.add_mode(mode[i]);
						}
					}
				} else {
					msg += ":" + config.get_server_name() + " 501 " + nickname + " :Unknown mode \"" + mode[i] + "\"\r\n";
				}
			}
		} else {
			reply = "Unknown mode";
			return 501;
		}
		if (!result_mode.empty()) {
			msg += ":" + c.to_string() + " MODE " + nickname + " :" + mode[0] + result_mode + "\r\n";
		}
		send(c.get_fd(), msg.c_str(), msg.size(), 0);
	}
	return 0;
}

int server::users_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	// using this hack to mute flags IT MUST BE REMOVED AFTER Implenting the function !!!!
	(void)cmd;
	(void)c;
	(void)reply;
	return 0;
}

int server::stats_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	// using this hack to mute flags IT MUST BE REMOVED AFTER Implenting the function !!!!
	(void)cmd;
	(void)c;
	(void)reply;
	return 0;
}

int server::info_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	// using this hack to mute flags IT MUST BE REMOVED AFTER Implenting the function !!!!
	(void)cmd;
	(void)c;
	(void)reply;
	return 0;
}

int server::invite_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	// using this hack to mute flags IT MUST BE REMOVED AFTER Implenting the function !!!!
	(void)cmd;
	(void)c;
	(void)reply;
	return 0;
}

int server::kick_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	// using this hack to mute flags IT MUST BE REMOVED AFTER Implenting the function !!!!
	(void)cmd;
	(void)c;
	(void)reply;
	return 0;
}

int server::names_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (cmd.get_args().size() > 1) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	}
	std::string msg;
	const std::string &server = config.get_server_name();
	const std::string &nick = c.get_nickname();
	if (cmd.get_args().empty()) {
		for (channel_map::const_iterator entry = channels.cbegin(); entry != channels.cend(); ++entry) {
			msg = ":" + server + " 353 " + nick + " = " + entry->first + " :" + entry->second.to_string() + "\r\n";
		}
		std::string without_channel = get_clients_without_channel();
		if (!without_channel.empty()) {
			msg += ":" + server + " 353 " + nick + " * * :" + without_channel + "\r\n";
		}
		msg += ":" + server + " 366 " + nick + " * :End of NAMES list\r\n";
	} else {
		const std::string &channel = cmd.get_args().at(0);
		if (channels.find(channel) != channels.end()) {
			msg = ":" + server + " 353 " + nick + " = " + channel + " :" + channels[channel].to_string() + "\r\n";
		}
		msg += ":" + server + " 366 " + nick + " " + channel + " :End of NAMES list\r\n";
	}
	send(c.get_fd(), msg.c_str(), msg.size(), 0);
	return 0;
}

int server::list_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	// using this hack to mute flags IT MUST BE REMOVED AFTER Implenting the function !!!!
	(void)cmd;
	(void)c;
	(void)reply;
	return 0;
}

int server::notice_cmd(const command_parser &cmd, client &c, std::string &) {
	if (cmd.get_args().size() == 2) {
		const std::string &receiver = cmd.get_args().at(0);
		const std::string &text = cmd.get_args().at(1);
		std::unordered_map<std::string, int>::iterator nicks_it = nick_to_fd.find(receiver);
		channel_map::iterator channels_it = channels.find(receiver);
		if (nicks_it != nick_to_fd.cend() || channels_it != channels.end()) {
			std::string msg = ":" + c.to_string() + " NOTICE " + receiver + " :" + text + "\r\n";
			if (channels_it != channels.end()) {
				channels_it->second.send_message(msg, &c);
			} else {
				send(nicks_it->second, msg.c_str(), msg.size(), 0);
			}
			return 0;
		}
	}
	return 0;
}

int server::ping_cmd(const command_parser &cmd, client &c, std::string &reply) {
	if (cmd.get_args().size() != 1) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	}
	std::string msg = ":" + config.get_server_name() + " PONG " + cmd.get_args().at(0) + "\r\n";
	send(c.get_fd(), msg.c_str(), msg.size(), 0);
	return 0;
}

int server::pong_cmd(const command_parser &cmd, client &, std::string &reply) {
	if (cmd.get_args().size() != 1) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	}
	// Nothing to do, PONG is just a reply to PING
	return 0;
}

int server::who_cmd(const command_parser &cmd, client &c, std::string &reply) {
	//TODO: implement flag 'o' for listing only operators
	if (cmd.get_args().size() != 1) {
		reply = cmd.get_cmd() + " :Syntax error";
		return 461;
	}
	const std::string &channel_name = cmd.get_args().at(0);
	channel_map::iterator it = channels.find(channel_name);
	(void)it;
	(void)c;
	//COMMAND: WHO On Progress...
	return 0;
}

int server::whois_cmd(const command_parser &cmd, client &c, std::string &reply) {
	// TODO
	// using this hack to mute flags IT MUST BE REMOVED AFTER Implenting the function !!!!
	(void)cmd;
	(void)c;
	(void)reply;
	return 0;
}
