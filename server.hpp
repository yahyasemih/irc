#ifndef SERVER_HPP
#define SERVER_HPP

#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <arpa/inet.h>
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
	bool is_running;

	void welcome_client(const client &c) const;
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
	int ping_cmd(const command_parser &cmd, client &c, std::string &reply);
	int who_cmd(const command_parser &cmd, client &c, std::string &reply);

	static command_map init_map();
public:
	server(int port, std::string password, std::string config_file = "");
	~server();

	void start();
	void stop();
};

#endif
