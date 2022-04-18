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
	std::string realname;
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
	void set_nickname(const std::string &nmame);
	const std::string &get_nickname() const;
	void set_username(const std::string &uname);
	const std::string &get_username() const;
	void set_realname(const std::string &rname);
	const std::string &get_realname() const;
	void set_pass(const std::string &p);
	const std::string &get_pass();

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
	if (command.back() == '\r') {
		return command.substr(0, command.size() - 1);
	} else {
		return command;
	}
}

void client::set_oper(bool oper) {
	this->oper = oper;
}

bool client::is_oper() const {
	return oper;
}

bool client::connection_already_registered() const {
	return false; // TODO: fix check
	return !pass.empty() || !nickname.empty() || !username.empty();
}

bool client::connection_not_registered() const {
	return false; // TODO: fix check
	return nickname.empty() && !username.empty();
}

bool client::is_connected() const {
	return connected;
}

void client::set_connected(bool connected) {
	if (connected && !this->connected) {
		send(get_fd(), ":irc.example.net 001 yahya :Welcome to the Internet Relay Network yahya!~yahya@localhost\r\n"
									":irc.example.net 002 yahya :Your host is irc.example.net, running version ngircd-26.1 (x86_64/apple/darwin18.7.0)\r\n"
									":irc.example.net 003 yahya :This server has been started Mon Mar 21 2022 at 21:25:26 (+01)\r\n"
									":irc.example.net 004 yahya irc.example.net ngircd-26.1 abBcCFiIoqrRswx abehiIklmMnoOPqQrRstvVz\r\n"
									":irc.example.net 005 yahya RFC2812 IRCD=ngIRCd CHARSET=UTF-8 CASEMAPPING=ascii PREFIX=(qaohv)~&@%+ CHANTYPES=#&+ CHANMODES=beI,k,l,imMnOPQRstVz CHANLIMIT=#&+:10 :are supported on this server\r\n"
									":irc.example.net 005 yahya CHANNELLEN=50 NICKLEN=9 TOPICLEN=490 AWAYLEN=127 KICKLEN=400 MODES=5 MAXLIST=beI:50 EXCEPTS=e INVEX=I PENALTY FNC :are supported on this server\r\n"
									":irc.example.net 251 yahya :There are 1 users and 0 services on 1 servers\r\n"
									":irc.example.net 254 yahya 1 :channels formed\r\n"
									":irc.example.net 255 yahya :I have 1 users, 0 services and 0 servers\r\n"
									":irc.example.net 265 yahya 1 1 :Current local users: 1, Max: 1\r\n"
									":irc.example.net 266 yahya 1 1 :Current global users: 1, Max: 1\r\n"
									":irc.example.net 250 yahya :Highest connection count: 1 (2 connections received)\r\n"
									":irc.example.net 422 yahya :MOTD file is missing\r\n", 1210, 0);
	}
	this->connected = connected;
}

void client::set_nickname(const std::string &nmame) {
	nickname = nmame;
}

const std::string &client::get_nickname() const {
	return nickname;
}

void client::set_username(const std::string &uname) {
	username = uname;
}

const std::string &client::get_username() const {
	return username;
}

void client::set_realname(const std::string &rname) {
	realname = rname;
}

const std::string &client::get_realname() const {
	return realname;
}

void client::set_pass(const std::string &p) {
	pass = p;
}

const std::string &client::get_pass() {
	return pass;
}

#endif
