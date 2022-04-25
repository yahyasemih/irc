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
	typedef std::unordered_map<std::string, std::string> operator_map;
private:
	const std::regex regex_clean_comments;
	const std::regex regex_is_key;
	const std::regex regex_parse_value;
	std::string server_name;
	std::string server_info;
	std::string version;
	std::string user_modes;
	std::string channel_modes;
	operator_map operators;
	std::string configs_dir;
	std::multimap<std::string, std::unordered_map<std::string, std::string> > config;
public:
	server_config();
	~server_config();

	const operator_map &get_operators() const;
	const std::string &get_server_name() const;
	const std::string &get_server_info() const;
	const std::string &get_version() const;
	const std::string &get_user_modes() const;
	const std::string &get_channel_modes() const;
	std::multimap<std::string, std::unordered_map<std::string, std::string> > &parse_conf();
};

#endif
