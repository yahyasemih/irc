#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>
#include <unordered_set>

#include "client.hpp"

class channel {
private:
	std::set<client *> users;
	std::unordered_set<client *> operators;
	std::unordered_set<client *> speakers;
	std::unordered_set<client *> invitees;
	std::string key;
	std::unordered_set<char> mode;
	size_t limit;
	std::string topic;

public:
	channel();
	channel(const std::string &key);
	~channel();

	void send_message(const std::string &msg, client *sender) const;
	const std::set<client *> &get_clients() const;
	void add_client(client *c);
	void remove_client(client *c);
	bool is_in_channel(client *c) const;
	bool can_speak(client *c) const;
	bool can_join(client *c) const;
	bool can_set_topic(client *c) const;
	bool can_invite(client *c) const;
	bool can_kick(client *c) const;
	size_t get_limit() const;
	size_t size() const;
	bool empty() const;
	std::string to_string() const;
	std::string get_mode() const;
	bool has_mode(char c) const;
	void add_mode(char c);
	void remove_mode(char c);
	void remove_invitation(client *c);
	void set_topic(const std::string &topic);
	const std::string &get_topic() const;
	std::string get_user_prefix(client *c) const;
};

#endif
