#include "user.hpp"

int User::id_counter = 0;

User::User() : user_id(++id_counter), nickname(""), password("") {}

User::User(const std::string& nick, const std::string& pass) : user_id(++id_counter), nickname(
		nick), password(pass) {
		DEBUG_MSG("New user registered,  nickname: " + nickname + ", " + "password: " + password);
}

int User::get_id() const noexcept {
	return user_id;
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

// void User::set_password(const std::string& password) noexcept {
//     password = password;
// }