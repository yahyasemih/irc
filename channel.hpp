#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

#include "client.hpp"

class channel {
private:
	std::set<client *> users;
	std::string key;
	client *owner;
public:
	channel();
	channel(const std::string &key);
	~channel();

	void send_message(const std::string &msg, client *sender) const;
	void add_client(client *c);
	void remove_client(client *c);
	bool is_in_channel(client *c) const;
	size_t size() const;
	bool empty() const;
	std::string to_string() const;
};

#endif
