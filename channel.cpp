#include "channel.hpp"

channel::channel() : created_at(std::chrono::system_clock::now().time_since_epoch().count() / 1000000) {
}

channel::channel(const std::string &key) : key(key),
		created_at(std::chrono::system_clock::now().time_since_epoch().count() / 1000000) {
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
	if (is_banned(c->get_nickname())) {
		return is_oper(c) || (speakers.find(c) != speakers.end());
	} else if (has_mode('m')) {
		return speakers.find(c) != speakers.end() || (!c->is_restricted() && operators.find(c) != operators.end());
	} else {
		return !has_mode('n') || is_in_channel(c);
	}
}

bool channel::can_join(client *c) const {
	return !has_mode('i') || invitees.find(c) != invitees.end()
			|| invite_list.find(c->get_nickname()) != invite_list.end();
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

bool channel::is_oper(client *c) const {
	return operators.find(c) != operators.cend();
}

size_t channel::get_limit() const {
	return has_mode('l') ? limit : std::numeric_limits<size_t>::max();
}

void channel::set_limit(size_t l) {
	limit = l;
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
	std::string args;
	for (std::unordered_set<char>::const_iterator it = mode.cbegin(); it != mode.cend(); ++it) {
		result += *it;
		if (*it == 'l') {
			args += " " + std::to_string(get_limit());
		} else if (*it == 'k') {
			args += " " + get_key();
		}
	}
	return result + args;
}

bool channel::has_mode(char c) const {
	return mode.find(c) != mode.end();
}

bool channel::add_mode(char c) {
	if (!has_mode(c)) {
		mode.insert(c);
		return true;
	}
	return false;
}

bool channel::remove_mode(char c) {
	if (has_mode(c)) {
		mode.erase(c);
		return true;
	}
	return false;
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

bool channel::add_oper(client *c) {
	return operators.insert(c).second;
}

bool channel::remove_oper(client *c) {
	if (operators.find(c) != operators.end()) {
		operators.erase(c);
		return true;
	}
	return false;
}

bool channel::add_speaker(client *c) {
	return speakers.insert(c).second;
}

bool channel::remove_speaker(client *c) {
	if (speakers.find(c) != speakers.end()) {
		speakers.erase(c);
		return true;
	}
	return false;
}

bool channel::add_ban(const std::string &str, std::string by) {
	if (ban_list.find(str) == ban_list.end()) {
		entry e;
		e.by = by;
		e.ts = std::chrono::system_clock::now().time_since_epoch().count() / 1000000;
		ban_list.insert(std::make_pair(str, e));
		return true;
	}
	return false;
}

bool channel::remove_ban(const std::string &str) {
	if (ban_list.find(str) != ban_list.end()) {
		ban_list.erase(str);
		return true;
	}
	return false;
}

bool channel::add_exception(const std::string &str, std::string by) {
	if (exception_list.find(str) == exception_list.end()) {
		entry e;
		e.by = by;
		e.ts = std::chrono::system_clock::now().time_since_epoch().count() / 1000000;
		exception_list.insert(std::make_pair(str, e));
		return true;
	}
	return false;
}

bool channel::remove_exception(const std::string &str) {
	if (exception_list.find(str) != exception_list.end()) {
		exception_list.erase(str);
		return true;
	}
	return false;
}

bool channel::add_invite(const std::string &str, std::string by) {
	if (invite_list.find(str) == invite_list.end()) {
		entry e;
		e.by = by;
		e.ts = std::chrono::system_clock::now().time_since_epoch().count() / 1000000;
		invite_list.insert(std::make_pair(str, e));
		return true;
	}
	return false;
}

bool channel::remove_invite(const std::string &str) {
	if (invite_list.find(str) != invite_list.end()) {
		invite_list.erase(str);
		return true;
	}
	return false;
}

const channel::ban_list_t &channel::get_ban_list() const {
	return ban_list;
}

const channel::exception_list_t &channel::get_exception_list() const {
	return exception_list;
}

const channel::invite_list_t &channel::get_invite_list() const {
	return invite_list;
}

void channel::set_key(const std::string &k) {
	key = k;
}

const std::string &channel::get_key() const {
	return key;
}

long long channel::get_created_at() const {
	return created_at;
}

bool channel::is_banned(const std::string &nickname) const {
	return ban_list.find(nickname) != ban_list.end() && exception_list.find(nickname) == exception_list.end();
}
