#pragma once 

#include "debug.hpp"

#include <string>

class User {
public:

    User();
	User(const std::string& nick, const std::string& pass);

	int         get_id() const noexcept;
	std::string get_nickname() const noexcept;
	std::string get_password() const noexcept;
	bool check_password(const std::string& pass) noexcept;
	// void set_password(const std::string& password) noexcept;

private:

	std::string nickname;
	std::string password;
	static int id_counter;
	int user_id;
};
