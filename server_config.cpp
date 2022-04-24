#include "server_config.hpp"

server_config::server_config() : server_name("irc.1337.ma"),
		server_info("This an IRC server made in 1337 school"),
		version("leet-irc 1.0.0"),
		user_modes("aioOrsw"),
		channel_modes("kmntv") {
	parse_conf();
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

const std::string &server_config::get_server_info() const {
	return server_info;
}

const std::string &server_config::get_version() const {
	return version;
}

const std::string &server_config::get_user_modes() const {
	return user_modes;
}

const std::string &server_config::get_channel_modes() const {
	return channel_modes;
}

void			   server_config::parse_conf() {
	std::ifstream inFile;
	inFile.open("irc.conf");

	std::stringstream strStream;
	strStream << inFile.rdbuf();
	std::string str = strStream.str();

	std::regex re("(#.*[\\s]*$)");
	str = std::regex_replace(str, re, "");

	strStream.str(str);
	std::string line;
	re = "^\\[([A-Za-z]+)\\]$";
	std::regex re2("^\\s*([^\\s;]\\S+)\\s*=\\s*([^#]*)$");
	std::string key = "";
	while (std::getline(strStream, line)) {
		if (std::regex_match(line, re)) {
			std::map<std::string, std::string> m;
			std::smatch match;
			std::regex_search(line, match, re);
			key = match[1];
			config[key] = m;
		} else if (std::regex_match(line, re2)) {
			std::smatch m;
			std::regex_search(line, m, re2);
			config[key][m[1]] = m[2];
		}
	}

	for (std::map<std::string, std::map<std::string, std::string> >::iterator it= config.begin(); it!= config.end(); ++it) {
		if (it->second.empty())
			continue;
		std::cout << it->first << std::endl;
		for (std::map<std::string, std::string>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
			std::cout << it2->first << " => " << it2->second << std::endl;
		}
	}
}
