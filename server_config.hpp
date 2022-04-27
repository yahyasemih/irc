#ifndef SERVER_CONFIG
#define SERVER_CONFIG

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <map>
#include <unordered_map>
#include <dirent.h>

class server_config {
public:
	typedef std::unordered_map<std::string, std::string> config_entry;
	typedef config_entry operator_map;
	typedef std::multimap<std::string, std::unordered_map<std::string, std::string> > config_map;
private:
	static const std::regex regex_clean_comments;
	static const std::regex regex_is_key;
	static const std::regex regex_parse_value;
	static const std::string configs_dir;

	std::string server_name;
	std::string server_info;
	std::string server_motd;
	std::string server_motd_file;
	std::string allowed_channels;
	std::string version;
	std::string user_modes;
	std::string channel_modes;
	operator_map operators;
	config_map config;
	void parse_conf();
	void set_conf();
public:
	server_config();
	~server_config();

	const operator_map &get_operators() const;
	const std::string &get_server_name() const;
	const std::string &get_server_info() const;
	const std::string &get_server_motd() const;
	const std::string &get_server_motd_file() const;
	const std::string &get_allowed_channels() const;
	const std::string &get_version() const;
	const std::string &get_user_modes() const;
	const std::string &get_channel_modes() const;
};

#endif
