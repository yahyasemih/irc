#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <poll.h>

#include <sstream>
#include <iostream>

class client {
private:
	int fd;
	std::string nickname;
	std::string username;
	std::string pass;
	bool oper;
	bool connected;
	std::stringstream stream;
public:
	client(int socket_fd);
	client(const client &o);
	~client();

	void receive(std::string str);
	int get_fd() const;
	std::stringstream &get_stream();
	bool has_command() const;
	std::string get_command();
	void set_oper(bool is_oper);
	bool is_oper() const;
	void set_pass(const std::string &p);
	const std::string &get_pass();
	void set_username(const std::string &uname);
	const std::string &get_username() const;
	void set_nickname(const std::string &nmame);
	const std::string &get_nickname() const;

	bool connection_already_registered() const;
	bool connection_not_registered() const;
	bool is_connected() const;
	void set_connected(bool connected);
};

client::client(int socket_fd) : fd(socket_fd), oper(false), connected(false) {
}

client::client(const client &o) : fd(o.fd), oper(o.oper), connected(o.connected) {
}

client::~client() {
}

void client::receive(std::string str) {
	stream << str;
}

int client::get_fd() const {
	return fd;
}

std::stringstream &client::get_stream() {
	return stream;
}

bool client::has_command() const {
	return stream.str().find("\n") != std::string::npos;
}

std::string client::get_command() {
	std::string command;
	getline(stream, command);
	stream.str(stream.str().substr(command.size() + 1));
	return command;
}

void client::set_oper(bool oper) {
	this->oper = oper;
}

bool client::is_oper() const {
	return oper;
}

void client::set_pass(const std::string &p) {
	pass = p;
}

const std::string &client::get_pass() {
	return pass;
}

bool client::connection_already_registered() const {
	return !pass.empty() || !nickname.empty() || !username.empty();
}

bool client::connection_not_registered() const {
	return nickname.empty() && !username.empty();
}

bool client::is_connected() const {
	return connected;
}

void client::set_connected(bool connected) {
	this->connected = connected;
}

void client::set_username(const std::string &uname) {
	username = uname;
}
const std::string &client::get_username() const {
	return username;
}
void client::set_nickname(const std::string &nmame) {
	nickname = nmame;
}
const std::string &client::get_nickname() const {
	return nickname;
}

#endif
