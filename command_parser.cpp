#include "command_parser.hpp"

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
	cmd = str.substr(cmd_start, idx - cmd_start);
	while (str[idx] == ' ') {
		++idx;
	}
	if (str[idx] == '\0') {
		return;
	}
	while (str[idx] != '\0' && str[idx] != ':') {
		while (str[idx] == ' ') {
			++idx;
		}
		size_t start_arg = idx;
		while (str[idx] != ' ' && str[idx] != '\0') {
			++idx;
		}
		args.push_back(str.substr(start_arg, idx - start_arg));
		while (str[idx] == ' ') {
			++idx;
		}
	}
	if (str[idx] == ':') {
		args.push_back(str.substr(idx + 1));
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

std::string command_parser::get_cmd_lowercase() const {
	std::string lower;
	std::transform(cmd.begin(), cmd.end(), std::back_inserter(lower), ::tolower);
	return lower;
}

const std::vector<std::string> &command_parser::get_args() const {
	return args;
}
