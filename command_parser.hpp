#ifndef COMMAND_PARSER_HPP
#define COMMAND_PARSER_HPP

#include <string>
#include <vector>
#include <algorithm>

enum command_type {
	NICK,
	USER,
	PASS,
	DIE,
	INVALID_CMD // TODO: complete list of commands
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
	const std::vector<std::string> &get_args() const;
};

command_parser::command_parser(const std::string &str) {
	size_t idx = 0;
	if (str[0] == ':') {
		while (str[idx] != ' ' && str[idx] != '\0') {
			++idx;
		}
		prefix = str.substr(0, idx);
	}
	if (str[idx] == '\0') {
		return;
	}
	while (str[idx] == ' ') {
		++idx;
	}
	size_t cmd_start = idx;
	while (str[idx] != ' ' && str[idx] != '\0') {
		++idx;
	}
	if (str[idx] == '\0') {
		return;
	}
	cmd = str.substr(cmd_start, idx - cmd_start);
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
	if (str[idx] == '\0') {
		return;
	}
	if (str[idx] == ':') {
		args.push_back(str.substr(idx));
	} else {
		std::stringstream stream(str.substr(idx));
		std::string arg;
		while (stream >> arg) {
			args.push_back(arg);
		}
	}
}

command_parser::~command_parser() {
}

const std::string &command_parser::get_prefix() const {
	return prefix;
}

const std::string &command_parser::get_cmd() const {
	return cmd;
}

const std::vector<std::string> &command_parser::get_args() const {
	return args;
}

#endif
