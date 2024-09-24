#include "user_repository.hpp"

UserRepository::UserRepository(DBManager& db_manager, const std::string& connection_name) : BaseRepository(db_manager), connection_name(connection_name) {}

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
        pqxx::result r = txn.exec_params("SELECT * FROM users WHERE id = $1", id);
        if (r.empty()) {
            return std::nullopt;
        }
        return constructUser(r[0]);
    } catch (const std::exception& e) {
        // add logs
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

std::optional<User> UserRepository::findByNickname(const std::string& nickname) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        pqxx::result r = txn.exec_params("SELECT * FROM users WHERE nickname = $1", nickname);
        if (r.empty()) {
            return std::nullopt;
        }
        return constructUser(r[0]);
    } catch (const std::exception& e) {
        // add logs
        return std::nullopt;
    }
}

User UserRepository::constructUser(const pqxx::row& row) {
    User user(row["nickname"].as<std::string>(), row["password"].as<std::string>());
    user.set_id(row["id"].as<int>());
    user.set_registered_timestamp(std::chrono::system_clock::from_time_t(row["registered_timestamp"].as<std::time_t>()));
    return user;
}