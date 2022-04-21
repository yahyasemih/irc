#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <poll.h>
#include <sys/socket.h>

#include <iostream>
#include <sstream>
#include <unordered_set>

class client {
private:
	int fd;
	std::string nickname;
	std::string username;
	std::string realname;
	std::string pass;
	std::string host;
	std::string away_msg;
	std::unordered_set<char> mode;
	bool oper;
	bool connected;
	std::stringstream stream;
public:
	client(int socket_fd, const std::string &host);
	client(const client &o);
	~client();

	void receive(std::string str);
	int get_fd() const;
	std::stringstream &get_stream();
	bool has_command() const;
	std::string get_command();
	void set_oper(bool is_oper);
	bool is_oper() const;
	void set_nickname(const std::string &nmame);
	const std::string &get_nickname() const;
	void set_username(const std::string &uname);
	const std::string &get_username() const;
	void set_realname(const std::string &rname);
	const std::string &get_realname() const;
	void set_pass(const std::string &p);
	const std::string &get_pass();
	const std::string &get_host() const;

	bool connection_already_registered() const;
	bool connection_not_registered() const;
	bool is_connected() const;
	void set_connected(bool connected);
	void set_away_msg(const std::string &msg);
	const std::string &get_away_msg() const;
	bool is_away() const;

	std::string to_string() const;
	std::string get_mode() const;
	bool has_mode(char c) const;
	void add_mode(char c);
	void remove_mode(char c);
};

#endif
