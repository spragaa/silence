#include "user_metadata_repository.hpp"

UserMetadataRepository::UserMetadataRepository(DBManager& db_manager, const std::string& connection_name) : BaseRepository(db_manager), connection_name(connection_name) {
    DEBUG_MSG("UserMetadataRepository created")
}

UserMetadataRepository::~UserMetadataRepository() = default;

int UserMetadataRepository::create(const User& user) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        
        auto format_timestamp = [](const Timestamp& ts) {
            auto time_t = std::chrono::system_clock::to_time_t(ts);
            std::stringstream ss;
            ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return ss.str();
        };

        pqxx::result r = txn.exec_params(
            "INSERT INTO users (nickname, password, registered_timestamp, last_online_timestamp, is_online) "
            "VALUES ($1, $2, $3, $4, $5) RETURNING id",
            user.get_nickname(),
            user.get_password(),
            format_timestamp(user.get_registered_timestamp()),
            format_timestamp(user.get_last_online_timestamp()),
            user.is_online()
        );
        
        DEBUG_MSG("Executing INSERT INTO users (nickname, password, registered_timestamp, last_online_timestamp, is_online) "
                  "VALUES ($1, $2, $3, $4, $5) RETURNING id\n"
                  "for user: " + user.to_json().dump());
        
        if (r.empty()) {
            DEBUG_MSG("[UserMetadataRepository::create] Failed to insert user");
            return false;
        }
        
        int inserted_id = r[0][0].as<int>();
        DEBUG_MSG("[UserMetadataRepository::create] User inserted successfully with id: " + std::to_string(inserted_id));
        
        txn.commit();
        return inserted_id;
    } catch (const std::exception& e) {
        DEBUG_MSG("[UserMetadataRepository::create] Exception caught: " + std::string(e.what()));
        return 0;
    }
}

std::optional<User> UserMetadataRepository::read(int id) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        pqxx::result r = txn.exec_params("SELECT * FROM USERS WHERE id = $1", id);
        DEBUG_MSG("executing SELECT * FROM USERS WHERE id = " + std::to_string(id));
        
        if (r.empty()) {
            DEBUG_MSG("[UserMetadataRepository::read] No user found with id: " + std::to_string(id));
            DEBUG_MSG("returning std::nullopt");
            return std::nullopt;
        }
        
        nlohmann::json json = pqxx_result_to_json(r);
        DEBUG_MSG("[UserMetadataRepository::read] response: " + json.dump());

        try {
            return construct_user(json);
        } catch (const std::exception& e) {
            DEBUG_MSG("[UserMetadataRepository::read] Error in construct_user: " + std::string(e.what()));
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        DEBUG_MSG("[UserMetadataRepository::read] Exception caught: " + std::string(e.what()));
        DEBUG_MSG("[UserMetadataRepository::read] No user found with id: " + std::to_string(id));
        DEBUG_MSG("returning std::nullopt");
        return std::nullopt;
    }
}

// didn't test
bool UserMetadataRepository::update(const User& user) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        
        auto format_timestamp = [](const Timestamp& ts) {
            auto time_t = std::chrono::system_clock::to_time_t(ts);
            std::stringstream ss;
            ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
            return ss.str();
        };

        pqxx::result r = txn.exec_params(
            "UPDATE users SET nickname = $1, password = $2, registered_timestamp = $3, "
            "last_online_timestamp = $4, is_online = $5 WHERE id = $6",
            user.get_nickname(),
            user.get_password(),
            format_timestamp(user.get_registered_timestamp()),
            format_timestamp(user.get_last_online_timestamp()),
            user.is_online(),
            user.get_id()
        );
        
        DEBUG_MSG("Executing UPDATE users SET nickname = $1, password = $2, registered_timestamp = $3, "
                  "last_online_timestamp = $4, is_online = $5 WHERE id = $6\n"
                  "for user: " + user.to_json().dump());
        
        if (r.affected_rows() == 0) {
            DEBUG_MSG("[UserMetadataRepository::update] No user updated");
            return false;
        }
        
        DEBUG_MSG("[UserMetadataRepository::update] User updated successfully with id: " + std::to_string(user.get_id()));
        
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        DEBUG_MSG("[UserMetadataRepository::update] Exception caught: " + std::string(e.what()));
        return false;
    }
}

// didn't test
bool UserMetadataRepository::remove(int id) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        pqxx::result r = txn.exec_params("DELETE FROM users WHERE id = $1", id);
        DEBUG_MSG("Executing DELETE FROM users WHERE id = $1\n"
                  "for user_id: " + std::to_string(id));

        
        if (r.affected_rows() == 0) {
            DEBUG_MSG("[UserMetadataRepository::remove] No user deleted");
            return false;
        }
        
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        DEBUG_MSG("[UserMetadataRepository::remove] Exception caught: " + std::string(e.what()));
        return false;
    }
}

bool UserMetadataRepository::authorize(int user_id, const std::string& nickname, const std::string& password) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        DEBUG_MSG("executing SELECT * FROM users WHERE id = " + std::to_string(user_id) + " AND nickname = " + nickname);
        
        pqxx::result r = txn.exec_params(
            "SELECT * FROM users WHERE id = $1 AND nickname = $2",
            user_id, nickname
        );

        if (r.empty()) {
            DEBUG_MSG("[UserMetadataRepository::authorize] No user found with id: " + std::to_string(user_id) + " and nickname: " + nickname);
            return false;
        }

        nlohmann::json json = pqxx_result_to_json(r);
        DEBUG_MSG("[UserMetadataRepository::authorize] response: " + json.dump());

        std::string stored_password = r[0][2].as<std::string>();
        if (password == stored_password) {
            DEBUG_MSG("[UserMetadataRepository::authorize] Password match for user id: " + std::to_string(user_id));
            return true;
        }

        DEBUG_MSG("[UserMetadataRepository::authorize] Password mismatch for user id: " + std::to_string(user_id));
        DEBUG_MSG("returning false");
        return false;
    } catch (const std::exception& e) {
        DEBUG_MSG("[UserMetadataRepository::authorize] Exception caught: " + std::string(e.what()));
        DEBUG_MSG("[UserMetadataRepository::authorize] Authorization failed for user id: " + std::to_string(user_id));
        return false;
    }
}

User UserMetadataRepository::construct_user(const nlohmann::json& user_json) {
    User user;
    return user.from_json(user_json);
}

nlohmann::json UserMetadataRepository::pqxx_result_to_json(const pqxx::result& r) const {
    nlohmann::json j;
    
    j["id"] = r[0][0].as<int>();
    j["nickname"] = r[0][1].as<std::string>();
    j["password"] = r[0][2].as<std::string>();
    j["registered_timestamp"] = r[0][3].as<std::string>();
    j["last_online_timestamp"] = r[0][4].as<std::string>();
    
    std::string is_online_str = r[0][5].as<std::string>();
    j["online"] = (is_online_str == "t");

    return j;
}