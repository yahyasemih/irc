#include "client.hpp"

client::client(int socket_fd, const std::string &host) : fd(socket_fd), host(host), oper(false), connected(false) {
}

client::client(const client &o) : fd(o.fd), nickname(o.nickname), username(o.username), realname(o.realname),
		pass(o.pass) , host(o.host), away_msg(o.away_msg), mode(o.mode), oper(o.oper), connected(o.connected),
		stream(o.stream.str()) {
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
	return is_connected();
}

bool client::connection_not_registered() const {
	return !is_connected();
}

bool client::is_connected() const {
	return connected;
}

void client::set_connected(bool connected) {
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

const std::string &client::get_host() const {
	return host;
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

bool client::is_restricted() const {
	return has_mode('r');
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
