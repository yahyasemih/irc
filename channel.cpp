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

const std::set<client *> &channel::get_clients() const {
	return this->users;
}

void channel::add_client(client *c) {
	if (users.find(c) == users.end()) {
		users.insert(c);
	}
	if (users.size() == 1) {
		operators.insert(c);
	}
}

void channel::remove_client(client *c) {
	operators.erase(c);
	speakers.erase(c);
	users.erase(c);
}

bool channel::is_in_channel(client *c) const {
	return users.find(c) != users.end();
}

bool channel::can_speak(client *c) const {
	if (has_mode('m')) {
		return speakers.find(c) != speakers.end() || (!c->is_restricted() && operators.find(c) != operators.end());
	} else {
		return !has_mode('n') || is_in_channel(c);
	}
}

bool channel::can_join(client *c) const {
	return !has_mode('i') || invitees.find(c) != invitees.end();
}

bool channel::can_set_topic(client *c) const {
	return !has_mode('t') || (!c->is_restricted() && operators.find(c) != operators.end());
}

bool channel::can_invite(client *c) const {
	return is_in_channel(c) && (!has_mode('i') || (!c->is_restricted() && operators.find(c) != operators.end()));
}

bool channel::can_kick(client *c) const {
	return (!c->is_restricted() && operators.find(c) != operators.end());
}

size_t channel::get_limit() const {
	return has_mode('l') ? limit : std::numeric_limits<size_t>::max();
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
			res += get_user_prefix(*it) + (*it)->get_nickname();
		} else {
			res += " " + get_user_prefix(*it) + (*it)->get_nickname();
		}
	}
	return res;
}

std::string channel::get_mode() const {
	std::string result = "+";
	for (std::unordered_set<char>::const_iterator it = mode.cbegin(); it != mode.cend(); ++it) {
		result += *it;
	}
	return result;
}

bool channel::has_mode(char c) const {
	return mode.find(c) != mode.end();
}

void channel::add_mode(char c) {
	if (!has_mode(c)) {
		mode.insert(c);
	}
}

void channel::remove_mode(char c) {
	if (has_mode(c)) {
		mode.erase(c);
	}
}

std::string channel::get_user_prefix(client *c) const {
	if (operators.find(c) != operators.end()) {
		return "@";
	} else if (speakers.find(c) != speakers.end()) {
		return "+";
	} else {
		return "";
	}
}

void channel::remove_invitation(client *c) {
	invitees.erase(c);
}

void channel::set_topic(const std::string &topic) {
	this->topic = topic;
}

const std::string &channel::get_topic() const {
	return topic;
}
