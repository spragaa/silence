#pragma once 

#include "debug.hpp"

#include <string>

class User {
public:

    User();
	User(const std::string& nick, const std::string& pass);

	int get_id() const noexcept;
	std::string get_nickname() const noexcept;
	std::string get_password() const noexcept;
	bool check_password(const std::string& pass) noexcept;
	void set_id(const int& user_id) noexcept;
	void set_nickname(const std::string& nick) noexcept;
	void set_password(const std::string& pass) noexcept;

private:

    // add more metadata?
	// registered at
	// last online
	// is online?
	static int user_id_counter;
	int id;
	std::string nickname;
	std::string password;	
};
