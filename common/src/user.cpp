#include "user.hpp"

namespace common {

int User::_user_id_counter = 1;

User::User() : _id(0), _nickname(""), _password("") {
}

User::User(const std::string& nick) : User() {
	_nickname = nick;
	INFO_MSG("New user created, nickname: " + _nickname);
}

User::User(const std::string& nick, const std::string& pass) : _id(_user_id_counter++), _nickname(
		nick), _password(pass), _registered_timestamp(std::chrono::system_clock::now()), _online(true) {
	// this log is missleading in case user already exists
	INFO_MSG("New user created, nickname: " + _nickname + ", " + "password: " + _password);
}

int User::get_id() const noexcept {
	return _id;
}

std::string User::get_nickname() const noexcept {
	return _nickname;
}

std::string User::get_password() const noexcept {
	return _password;
}

Timestamp User::get_registered_timestamp() const noexcept {
	return _registered_timestamp;
}

Timestamp User::get_last_online_timestamp() const noexcept {
	return _last_online_timestamp;
}

bool User::is_online() const noexcept {
	return _online;
}

bool User::check_password(const std::string& pass) noexcept {
	return pass == _password;
}

void User::set_id(const int& user_id) noexcept {
	_id = user_id;
}

void User::set_nickname(const std::string& nick) noexcept {
	_nickname = nick;
}

void User::set_password(const std::string& pass) noexcept {
	_password = pass;
}

void User::set_registered_timestamp(const Timestamp& timestamp) noexcept {
	_registered_timestamp = timestamp;
}

void User::set_last_online_timestamp(const Timestamp& timestamp) noexcept {
	_last_online_timestamp = timestamp;
}

void User::set_online(const bool is_online) noexcept {
	_online = is_online;
}

nlohmann::json User::to_json() const {
	nlohmann::json j;
	j["id"] = _id;
	j["nickname"] = _nickname;
	j["password"] = _password;
	j["registered_timestamp"] = _registered_timestamp.time_since_epoch().count();
	j["last_online_timestamp"] = _last_online_timestamp.time_since_epoch().count();
	j["online"] = _online;

	return j;
}

User User::from_json(const nlohmann::json& j) {
	User user;
	user._id = j["id"];
	user._nickname = j["nickname"];
	user._password = j["password"];
	// Exception in Client::run() : [json.exception.type_error.302] type must be string, but is number
	// user._registered_timestamp = parse_timestamp(j["registered_timestamp"]);
	// user._last_online_timestamp = parse_timestamp(j["last_online_timestamp"]);
	user._online = j["online"].get<bool>();

	return user;
}

Timestamp User::parse_timestamp(const std::string& timestamp_str) {
	std::tm tm = {};
	std::istringstream ss(timestamp_str);
	ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
	auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
	return std::chrono::time_point_cast<std::chrono::system_clock::duration>(tp);
}

void User::save_user_data_to_json(const std::string& filename) const {
	nlohmann::json j = to_json();

	std::filesystem::path filePath(filename);
	std::filesystem::path dir = filePath.parent_path();

	try {
		if (!std::filesystem::exists(dir)) {
			if (std::filesystem::create_directories(dir)) {
				DEBUG_MSG("Created directory: " + dir.string());
			} else {
				ERROR_MSG("Failed to create directory: " + dir.string());
				return;
			}
		}
	} catch (const std::filesystem::filesystem_error& e) {
		ERROR_MSG("Filesystem error: " + std::string(e.what()));
		return;
	}

	std::ofstream file(filename);
	if (file.is_open()) {
		file << j.dump(4);
		file.close();
		DEBUG_MSG("User data saved to " + filename);
	} else {
		DEBUG_MSG("Unable to open file for writing: " + filename);
	}
}

User User::load_user_data_from_json(const std::string& filename) {
	std::ifstream file(filename);
	if (file.is_open()) {
		nlohmann::json j;
		file >> j;
		file.close();
		DEBUG_MSG("User data loaded from " + filename);
		return from_json(j);
	} else {
		DEBUG_MSG("Unable to open file for reading: " + filename);
		return User();
	}
}

}