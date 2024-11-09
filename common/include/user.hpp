#pragma once

#include "debug.hpp"
#include "common.hpp"
#include <nlohmann/json.hpp>

#include <string>
#include <filesystem>
#include <fstream>

namespace common {

class User {
public:
	User();
	User(const std::string& nick);
	User(const std::string& nick, const std::string& pass);

	int get_id() const noexcept;
	std::string get_nickname() const noexcept;
	std::string get_password() const noexcept;
	Timestamp get_registered_timestamp() const noexcept;
	Timestamp get_last_online_timestamp() const noexcept;
	bool is_online() const noexcept;

	bool check_password(const std::string& pass) noexcept;
	void set_id(const int& user_id) noexcept;
	void set_nickname(const std::string& nick) noexcept;
	void set_password(const std::string& pass) noexcept;
	void set_registered_timestamp(const Timestamp& timestamp) noexcept;
	void set_last_online_timestamp(const Timestamp& timestamp) noexcept;
	void set_online(const bool is_online) noexcept;

	void save_user_data_to_json(const std::string& filename) const;
	static User load_user_data_from_json(const std::string& filename);

	nlohmann::json to_json() const;
	static User from_json(const nlohmann::json& j);
	static Timestamp parse_timestamp(const std::string& timestamp_str);

private:
	static int _user_id_counter; // should rm this logic, or should I?
	int _id;
	std::string _nickname;
	std::string _password;
	Timestamp _registered_timestamp;
	Timestamp _last_online_timestamp;
	bool _online;
};

}