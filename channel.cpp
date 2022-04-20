#include "channel.hpp"

channel::channel() {
}

channel::channel(const std::string &key) : key(key) {
}

channel::~channel() {
}

void channel::send_message(const std::string &msg, client *sender) const {
	for (std::set<client *>::iterator it = users.begin(); it != users.end(); ++it) {
		if (*it != sender) {
			send((*it)->get_fd(), msg.c_str(), msg.size(), 0);
		}
	}
}

void channel::add_client(client *c) {
	if (users.find(c) == users.end()) {
		users.insert(c);
	}
	if (users.size() == 1) {
		owner = c;
	}
}

void channel::remove_client(client *c) {
	if (owner == c) {
		owner = nullptr;
	}
	users.erase(c);
}

bool channel::is_in_channel(client *c) const {
	return users.find(c) != users.end();
}

size_t channel::size() const {
	return users.size();
}

bool channel::empty() const {
	return users.empty();
}

std::string channel::to_string() const {
	std::string res;
	for (std::set<client *>::const_iterator it = users.cbegin(); it != users.cend(); ++it) {
		if (res.empty()) {
			res += (*it == owner ? "@" : "") + (*it)->get_nickname();
		} else {
			res += (*it == owner ? " @" : " ") + (*it)->get_nickname();
		}
	}
	// TODO: check what else should be printed in addition to '@' before channel creator
	return res;
}
