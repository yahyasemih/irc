#include "server_config.hpp"

const std::regex server_config::regex_clean_comments = std::regex("(#.*[\\s]*$)");
const std::regex server_config::regex_is_key = std::regex("^\\[([A-Za-z]+)\\]$");
const std::regex server_config::regex_parse_value("^\\s*([^\\s;]\\S+)\\s*=\\s*\"([^\"]*).*$");
const std::string server_config::configs_dir = "./config";

server_config::server_config() :
		server_name("irc.1337.ma"),
		server_info("This an IRC server made in 1337 school"),
		server_motd(""),
		server_motd_file(""),
		allowed_channels("#&+"),
		version("leet-irc 1.0.0"),
		user_modes("aioOrsw"),
		channel_modes("aovimntklbeI") {
			parse_conf();
			set_conf();
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

const std::string &server_config::get_server_motd() const {
	return server_motd;
}

const std::string &server_config::get_server_motd_file() const {
	return server_motd_file;
}

const std::string &server_config::get_allowed_channels() const {
	return allowed_channels;
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

static void read_conf_folder(const std::string &configs_dir, std::string &str) {
	DIR *dir;
	dirent *ent;

	if ((dir = opendir(configs_dir.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::regex regex_config_names("^.*\\.conf$");
			if (std::regex_match(ent->d_name, regex_config_names)) {
				std::ifstream inFile;
				std::stringstream strStream;
				inFile.open(configs_dir + "/" + ent->d_name);
				strStream << inFile.rdbuf();
				str += strStream.str();
			}
		}
		closedir(dir);
	}
}

void server_config::parse_conf() {

	std::string str;
	std::stringstream strStream;

	read_conf_folder(configs_dir, str);
	str = std::regex_replace(str, regex_clean_comments, "");

	strStream.str(str);
	std::string line;
	std::string key = "";
	while (std::getline(strStream, line)) {
		if (std::regex_match(line, regex_is_key)) {
			std::unordered_map<std::string, std::string> m;
			std::smatch match;
			std::regex_search(line, match, regex_is_key);
			key = match[1];
			config.insert(std::make_pair(key, m));
		} else if (std::regex_match(line, regex_parse_value)) {
			std::smatch m;
			std::regex_search(line, m, regex_parse_value);
			if (config.find(key) != config.end()) {
				(--config.equal_range(key).second)->second[m[1]] = m[2];
			}
		}
	}
}

void server_config::set_conf() {
	for (config_map::iterator it= config.begin(); it!= config.end(); ++it) {
		if (it->second.empty()) {
			continue;
		} else if (it->first == "Operator") {
			config_entry::iterator name = it->second.find("Name");
			config_entry::iterator pass = it->second.find("Password");
			if (name != it->second.end() && pass != it->second.end()) {
				operators.insert(std::make_pair(name->second, pass->second));
			}
		} else if (it->first == "Global") {
			config_entry::iterator sname = it->second.find("Name");
			config_entry::iterator sinfo = it->second.find("Info");
			config_entry::iterator smotd = it->second.find("MotdPhrase");
			config_entry::iterator smotd_file = it->second.find("MotdFile");
			if (sname != it->second.end()) {
				server_name = sname->second;
			}
			if (sinfo != it->second.end()) {
				server_info = sinfo->second;
			}
			if (smotd != it->second.end()) {
				server_motd = smotd->second;
			}
			if (smotd_file != it->second.end()) {
				server_motd_file = smotd_file->second;
			}
		} else if (it->first == "Options") {
			config_entry::iterator allowed_channel_types = it->second.find("AllowedChannelTypes");
			if (allowed_channel_types != it->second.end()) {
				allowed_channels = allowed_channel_types->second;
			}
		}
	}
}
