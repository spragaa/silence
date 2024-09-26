#include "user_repository.hpp"

UserRepository::UserRepository(DBManager& db_manager, const std::string& connection_name) : BaseRepository(db_manager), connection_name(connection_name) {
    DEBUG_MSG("UserRepository created")
}

UserRepository::~UserRepository() = default;

bool UserRepository::create(const User& user) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        txn.exec_params(
            "INSERT INTO users (nickname, password, registered_timestamp) "
            "VALUES ($1, $2, $3)",
            user.get_nickname(),
            user.get_password(),
            std::chrono::system_clock::to_time_t(user.get_registered_timestamp())
        );
        txn.commit();
        return true;
    } catch (const std::exception& e) {
        // add logs
        return false;
    }
}

std::optional<User> UserRepository::read(int id) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        pqxx::result r = txn.exec_params("SELECT * FROM USERS WHERE id = \$1", id);
        DEBUG_MSG("executing SELECT * FROM USERS WHERE id = " + std::to_string(id));
        
        if (r.empty()) {
            DEBUG_MSG("[UserRepository::read] No user found with id: " + std::to_string(id));
            DEBUG_MSG("[UserRepository::read] returning std::nullopt");
            return std::nullopt;
        }
        
        nlohmann::json json = pqxx_result_to_json(r);
        DEBUG_MSG("[UserRepository::read] response: " + json.dump());

        try {
            return construct_user(json);
        } catch (const std::exception& e) {
            DEBUG_MSG("[UserRepository::read] Error in construct_user: " + std::string(e.what()));
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        DEBUG_MSG("[UserRepository::read] Exception caught: " + std::string(e.what()));
        DEBUG_MSG("[UserRepository::read] returning std::nullopt");
        return std::nullopt;
    }
}

bool UserRepository::update(const User& user) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        pqxx::result r = txn.exec_params(
            "UPDATE users SET nickname = $1, password = $2 WHERE id = $3",
            user.get_nickname(),
            user.get_password(),
            user.get_id()
        );
        txn.commit();
        return r.affected_rows() > 0;
    } catch (const std::exception& e) {
        // add logs
        return false;
    }
}

bool UserRepository::remove(int id) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        pqxx::result r = txn.exec_params("DELETE FROM users WHERE id = $1", id);
        txn.commit();
        return r.affected_rows() > 0;
    } catch (const std::exception& e) {
        // add logs
        return false;
    }
}

// std::optional<User> UserRepository::findByNickname(const std::string& nickname) {
//     try {
//         pqxx::work txn(db_manager.get_connection(connection_name));
//         pqxx::result r = txn.exec_params("SELECT * FROM users WHERE nickname = $1", nickname);
//         if (r.empty()) {
//             return std::nullopt;
//         }
//         return construct_user(r[0]);
//     } catch (const std::exception& e) {
//         // add logs
//         return std::nullopt;
//     }
// }

User UserRepository::construct_user(const nlohmann::json& user_json) {
    User user;
    return user.from_json(user_json);
}

nlohmann::json UserRepository::pqxx_result_to_json(const pqxx::result& r) const {
    nlohmann::json j;
    
    j["id"] = r[0][0].as<int>();
    j["nickname"] = r[0][1].as<std::string>();
    j["password"] = r[0][2].as<std::string>();
    j["registered_timestamp"] = r[0][3].as<std::string>();
    j["last_online_timestamp"] = r[0][4].as<std::string>();
    
    std::string is_online_str = r[0][5].as<std::string>();
    j["is_online"] = (is_online_str == "t");

    return j;
}