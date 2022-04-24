#include "server_config.hpp"

server_config::server_config() : server_name("irc.1337.ma"),
		server_info("This an IRC server made in 1337 school"),
		version("leet-irc 1.0.0"),
		user_modes("aioOrsw"),
		channel_modes("ovimntklbeI"),
		configs_dir("./config") {
			parse_conf();
			//this for loop just temporarely to see results
			for (std::map<std::string, std::map<std::string, std::string> >::iterator it= config.begin(); it!= config.end(); ++it) {
				if (it->second.empty())
					continue;
				std::cout << "==========" << it->first << "==========" << std::endl;
				for (std::map<std::string, std::string>::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
					std::cout << it2->first << " => " << it2->second << std::endl;
				}
			}
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

void				read_conf_folder(const std::string &configs_dir, std::string &str) {
	DIR		*dir;
	struct	dirent *ent;

	std::stringstream strStream;

	if ((dir = opendir(configs_dir.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::regex regex_config_names("^.*\\.conf$");
			if (std::regex_match(ent->d_name, regex_config_names)) {
				std::ifstream inFile;
				inFile.open(configs_dir + "/" + ent->d_name);
				strStream << inFile.rdbuf();
				str += strStream.str();
			}
		}
		closedir(dir);
	}
}

std::map<std::string, std::map<std::string, std::string> > &server_config::parse_conf() {

	std::string str;
	std::stringstream strStream;

	read_conf_folder(configs_dir, str);
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
	return config;
}
