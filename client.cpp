#include "client.hpp"

client::client(int socket_fd, const std::string &host) : fd(socket_fd), host(host), oper(false), connected(false) {
}

client::client(const client &o) : fd(o.fd), host(o.host), oper(o.oper), connected(o.connected) {
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
		std::string msg = ":irc.example.net 001 " + get_nickname() + " :Welcome to the Internet Relay Network " + to_string() + "\r\n"
				":irc.example.net 002 " + get_nickname() + " :Your host is irc.example.net, running version ngircd-26.1 (x86_64/apple/darwin18.7.0)\r\n"
				":irc.example.net 003 " + get_nickname() + " :This server has been started Mon Mar 21 2022 at 21:25:26 (+01)\r\n"
				":irc.example.net 004 " + get_nickname() + " irc.example.net ngircd-26.1 abBcCFiIoqrRswx abehiIklmMnoOPqQrRstvVz\r\n"
				//":irc.example.net 005 " + get_nickname() + " RFC2812 IRCD=ngIRCd CHARSET=UTF-8 CASEMAPPING=ascii PREFIX=(qaohv)~&@%+ CHANTYPES=#&+ CHANMODES=beI,k,l,imMnOPQRstVz CHANLIMIT=#&+:10 :are supported on this server\r\n"
				//":irc.example.net 005 " + get_nickname() + " CHANNELLEN=50 NICKLEN=9 TOPICLEN=490 AWAYLEN=127 KICKLEN=400 MODES=5 MAXLIST=beI:50 EXCEPTS=e INVEX=I PENALTY FNC :are supported on this server\r\n"
				":irc.example.net 251 " + get_nickname() + " :There are 1 users and 0 services on 1 servers\r\n";
				//":irc.example.net 254 " + get_nickname() + " 1 :channels formed\r\n"
				//":irc.example.net 255 " + get_nickname() + " :I have 1 users, 0 services and 0 servers\r\n"
				//":irc.example.net 265 " + get_nickname() + " 1 1 :Current local users: 1, Max: 1\r\n"
				//":irc.example.net 266 " + get_nickname() + " 1 1 :Current global users: 1, Max: 1\r\n"
				//":irc.example.net 250 " + get_nickname() + " :Highest connection count: 1 (2 connections received)\r\n"
				//":irc.example.net 422 " + get_nickname() + " :MOTD file is missing\r\n";
		send(get_fd(), msg.c_str(), msg.size(), 0);
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

std::string client::to_string() const {
	return nickname + "!~" + username + "@" + host;
}

void client::set_away_msg(const std::string &msg) {
	away_msg = msg;
	if (msg.empty()) {
		remove_mode('a');
	} else {
		add_mode('a');
	}
}
const std::string &client::get_away_msg() const {
	return away_msg;
}

bool client::is_away() const {
	return !away_msg.empty() || has_mode('a');
}

std::string client::get_mode() const {
	std::string result = "+";
	for (std::unordered_set<char>::const_iterator it = mode.cbegin(); it != mode.cend(); ++it) {
		result += *it;
	}
	return result;
}

bool client::has_mode(char c) const {
	return mode.find(c) != mode.end();
}

void client::add_mode(char c) {
	if (!has_mode(c)) {
		mode.insert(c);
	}
}

void client::remove_mode(char c) {
	if (has_mode(c)) {
		mode.erase(c);
	}
}
