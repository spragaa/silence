#include "user.hpp"

int User::id_counter = 0;

User::User(const std::string& nickname) : user_id(++id_counter), nickname(
		nickname) {
}

int User::get_id() const noexcept {
	return user_id;
}

std::string User::get_nickname() const noexcept {
	return nickname;
}
