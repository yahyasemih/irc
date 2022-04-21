#ifndef SERVER_CONFIG
#define SERVER_CONFIG

#include <string>
#include <unordered_map>

class server_config {
public:
	typedef std::unordered_map<std::string, std::string> operator_map;
private:
	std::string server_name;
	std::string server_info;
	std::string version;
	std::string user_modes;
	std::string channel_modes;
	operator_map operators;
public:
	server_config();
	server_config(const std::string &config_file);
	~server_config();

	const operator_map &get_operators() const;
	const std::string &get_server_name() const;
	const std::string &get_version() const;
	const std::string &get_user_modes() const;
	const std::string &get_channel_modes() const;
};

#endif
