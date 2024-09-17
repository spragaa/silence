#include "user.hpp"

int User::user_id_counter = 0;

User::User() : id(), nickname(""), password("") {}

User::User(const std::string& nick, const std::string& pass) : id(user_id_counter++), nickname(
		nick), password(pass) {
		DEBUG_MSG("New user registered,  nickname: " + nickname + ", " + "password: " + password);
}

int User::get_id() const noexcept {
    return id;
}

std::string User::get_nickname() const noexcept {
	return nickname;
}

std::string User::get_password() const noexcept {
	return password;
}

bool User::check_password(const std::string& pass) noexcept {
    return pass == password;
}

void User::set_id(const int& user_id) noexcept {
    id = id;
}

void User::set_nickname(const std::string& nick) noexcept {
    nickname = nick;
}

void User::set_password(const std::string& pass) noexcept {
    password = pass;
}
