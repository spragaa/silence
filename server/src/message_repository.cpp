#include "message_repository.hpp"

MessageRepository::MessageRepository(DBManager& db_manager, const std::string& connection_name) : BaseRepository(db_manager), connection_name(connection_name) {
    DEBUG_MSG("MessageRepository created")
}
MessageRepository::~MessageRepository() = default;

int MessageRepository::create(const Message& message) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        txn.exec_params(
            "INSERT INTO messages (sender_id, receiver_id, text, created_timestamp) "
            "VALUES ($1, $2, $3, $4)",
            message.get_sender_id(),
            message.get_receiver_id(),
            message.get_text(),
            std::chrono::system_clock::to_time_t(message.get_created_timestamp())
        );
        txn.commit();
        return 1;
    } catch (const std::exception& e) {
        // Log the error
        return 0;
    }
}

std::optional<Message> MessageRepository::read(int id) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        pqxx::result r = txn.exec_params("SELECT * FROM messages WHERE id = $1", id);
        if (r.empty()) {
            return std::nullopt;
        }
        return constructMessage(r[0]);
    } catch (const std::exception& e) {
        // Log the error
        return std::nullopt;
    }
}

bool MessageRepository::update(const Message& message) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        pqxx::result r = txn.exec_params(
            "UPDATE messages SET text = $1, last_edited_timestamp = $2 "
            "WHERE id = $3",
            message.get_text(),
            std::chrono::system_clock::to_time_t(message.get_last_edited_timestamp().value_or(std::chrono::system_clock::now())),
            message.get_id()
        );
        txn.commit();
        return r.affected_rows() > 0;
    } catch (const std::exception& e) {
        // Log the error
        return false;
    }
}

bool MessageRepository::remove(int id) {
    try {
        pqxx::work txn(db_manager.get_connection(connection_name));
        pqxx::result r = txn.exec_params(
            "UPDATE messages SET deleted = TRUE, deleted_timestamp = $1 "
            "WHERE id = $2",
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()),
            id
        );
        txn.commit();
        return r.affected_rows() > 0;
    } catch (const std::exception& e) {
        // Log the error
        return false;
    }
}

Message MessageRepository::constructMessage(const pqxx::row& row) {
    Message msg(
        row["sender_id"].as<int>(),
        row["receiver_id"].as<int>(),
        row["text"].as<std::string>()
    );
    // implement set these values in the Message class
    // msg.set_id(row["id"].as<int>());
    // msg.set_deleted(row["deleted"].as<bool>());
    // msg.set_created_timestamp(std::chrono::system_clock::from_time_t(row["created_timestamp"].as<std::time_t>()));
    // if (!row["deleted_timestamp"].is_null())
    //     msg.set_deleted_timestamp(std::chrono::system_clock::from_time_t(row["deleted_timestamp"].as<std::time_t>()));
    // if (!row["last_edited_timestamp"].is_null())
    //     msg.set_last_edited_timestamp(std::chrono::system_clock::from_time_t(row["last_edited_timestamp"].as<std::time_t>()));
    return msg;
}