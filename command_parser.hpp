#ifndef COMMAND_PARSER_HPP
#define COMMAND_PARSER_HPP

#include <string>
#include <vector>
#include <algorithm>

enum command_type {
	NICK,
	USER,
	PASS,
	OPER,
	DIE,
	PRIVMSG,
	JOIN,
	QUIT,
	PART,
	AWAY,
	TOPIC,
	MODE,
	USERS,
	INFO,
	INVITE,
	KICK,
	NAMES,
	LIST,
	NOTICE,
	PING,
	PONG,
	WHO,
	WHOIS,
	ISON,
	INVALID_CMD
};

class command_parser {
private:
	std::string prefix;
	std::string cmd;
	std::vector<std::string> args;
public:
	command_parser(const std::string &str);
	~command_parser();

	const std::string &get_prefix() const;
	const std::string &get_cmd() const;
	std::string get_cmd_lowercase() const;
	const std::vector<std::string> &get_args() const;
};

#endif
