#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

#include "client.hpp"

class channel {
private:
	std::set<client *> users;
	std::string key;
public:
	channel();
	channel(const std::string &key);
	~channel();

	void send_message(const std::string &msg, client *sender) const;
	void add_client(client *c);
	void remove_client(client *c);
	bool is_in_channel(client *c) const;
};

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
}

void channel::remove_client(client *c) {
	users.erase(c);
}

bool channel::is_in_channel(client *c) const {
	return users.find(c) != users.end();
}

#endif
