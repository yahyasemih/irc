#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <chrono>
#include <string>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "client.hpp"

struct entry {
	std::string by;
	long long ts;
};

class channel {
private:
	std::set<client *> users;
	std::unordered_set<client *> operators;
	std::unordered_set<client *> speakers;
	std::unordered_set<client *> invitees;
	std::unordered_map<std::string, entry> ban_list;
	std::unordered_map<std::string, entry> exception_list;
	std::unordered_map<std::string, entry> invite_list;
	std::string key;
	std::unordered_set<char> mode;
	size_t limit;
	std::string topic;
	long long created_at;

public:
	typedef std::unordered_map<std::string, entry> ban_list_t;
	typedef std::unordered_map<std::string, entry> exception_list_t;
	typedef std::unordered_map<std::string, entry> invite_list_t;
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
	bool is_oper(client *c) const;
	size_t get_limit() const;
	void set_limit(size_t l);
	size_t size() const;
	bool empty() const;
	std::string to_string() const;
	std::string get_mode() const;
	bool has_mode(char c) const;
	bool add_mode(char c);
	bool remove_mode(char c);
	void remove_invitation(client *c);
	void set_topic(const std::string &topic);
	const std::string &get_topic() const;
	std::string get_user_prefix(client *c) const;
	bool add_oper(client *c);
	bool remove_oper(client *c);
	bool add_speaker(client *c);
	bool remove_speaker(client *c);
	bool add_ban(const std::string &str, std::string by);
	bool remove_ban(const std::string &str);
	bool add_exception(const std::string &str, std::string by);
	bool remove_exception(const std::string &str);
	bool add_invite(const std::string &str, std::string by);
	bool remove_invite(const std::string &str);
	const ban_list_t &get_ban_list() const;
	const exception_list_t &get_exception_list() const;
	const invite_list_t &get_invite_list() const;
	void set_key(const std::string &k);
	const std::string &get_key() const;
	long long get_created_at() const;
	bool is_banned(client *c) const;
	bool is_anonymous() const;
};

#endif
