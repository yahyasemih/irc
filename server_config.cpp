#include "server_config.hpp"

server_config::server_config() : server_name(":irc.1337.ma"), server_info("This an IRC server made in 1337 school") {
}

server_config::server_config(const std::string &config_file) {
	//TODO: parse config file
	//this hack to mute flags MUST BE DELETE AFTER Implementing the function
	(void)config_file;
}

server_config::~server_config() {
}

const server_config::operator_map &server_config::get_operators() const {
	return operators;
}

const std::string &server_config::get_server_name() const {
	return server_name;
}
