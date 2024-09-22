#include "user.hpp"

int User::user_id_counter = 1;

User::User() : id(0), nickname(""), password("") {}

User::User(const std::string& nick) : User() {
    nickname = nick;
}

User::User(const std::string& nick, const std::string& pass) : id(user_id_counter++), nickname(
		nick), password(pass), registered_timestamp(std::chrono::system_clock::now()) {
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

Timestamp User::get_registered_timestamp() const noexcept {
    return registered_timestamp;
}

bool User::check_password(const std::string& pass) noexcept {
    return pass == password;
}

void User::set_id(const int& user_id) noexcept {
    id = user_id;
}

void User::set_nickname(const std::string& nick) noexcept {
    nickname = nick;
}

void User::set_password(const std::string& pass) noexcept {
    password = pass;
}

void User::set_registered_timestamp(const Timestamp& timestamp) noexcept {
    registered_timestamp = timestamp;
}

nlohmann::json User::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["nickname"] = nickname;
    j["password"] = password;
    j["registered_timestamp"] = registered_timestamp.time_since_epoch().count();
    return j;
}

User User::from_json(const nlohmann::json& j) {
    User user;
    user.id = j["id"];
    user.nickname = j["nickname"];
    user.password = j["password"];
    user.registered_timestamp = Timestamp(std::chrono::nanoseconds(j["registered_timestamp"].get<int64_t>()));
    return user;
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
                DEBUG_MSG("Failed to create directory: " + dir.string());
                return;
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        DEBUG_MSG("Filesystem error: " + std::string(e.what()));
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