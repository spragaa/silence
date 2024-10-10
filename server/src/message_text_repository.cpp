#include "message_text_repository.hpp"
#include <regex>

MessageTextRepository::MessageTextRepository(const std::string& connection_string) {
    try {
        DEBUG_MSG("[MessageTextRepository::MessageTextRepository] Parsing message_text database connection string: " + connection_string);
        auto connection_options = parse_config_string(connection_string);
        redis = std::make_unique<sw::redis::Redis>(connection_options);
        INFO_MSG("[MessageTextRepository::MessageTextRepository] Successfully connected to message_text Redis database");
    } catch (const std::exception& e) {
        std::cerr << "[MessageTextRepository::MessageTextRepository] Error: " << e.what() << std::endl;
        FATAL_MSG("[MessageTextRepository::MessageTextRepository] Failed to connect to Redis database");
    }
}

sw::redis::ConnectionOptions MessageTextRepository::parse_config_string(const std::string& connection_string) {
    sw::redis::ConnectionOptions options;

    std::regex connection_regex("redis://(?:(\\w+):)?([^@]+)@([\\w.]+):(\\d+)(?:/(\\d+))?");
    std::smatch matches;

    if (std::regex_match(connection_string, matches, connection_regex)) {
        if (matches[1].matched) {
            options.user = matches[1].str();
        }
        options.password = matches[2].str();
        options.host = matches[3].str();
        options.port = std::stoi(matches[4].str());
        if (matches[5].matched) {
            options.db = std::stoi(matches[5].str());
        }
    } else {
        throw std::invalid_argument("[MessageTextRepository::parse_config_string] Invalid connection string format");
    }

    DEBUG_MSG("[MessageTextRepository::parseConnectionString] Parsed successfully!");

    return options;
}

int MessageTextRepository::create(const MessageText& message) {
    try {
        int id = get_next_id();
        std::string key = "message:" + std::to_string(id);
        
        auto json = message.to_json();
        json["id"] = id;
        
        redis->set(key, json.dump());
        DEBUG_MSG("[MessageTextRepository::create] Created message with ID: " + std::to_string(id));
        return id;
    } catch (const std::exception& e) {
        ERROR_MSG("[MessageTextRepository::create] Error: " + std::string(e.what()));
        return -1;
    }
}

std::optional<MessageText> MessageTextRepository::read(int id) {
    try {
        std::string key = "message:" + std::to_string(id);
        auto value = redis->get(key);
        
        if (value) {
            auto json = nlohmann::json::parse(*value);
            MessageText message;
            message = message.from_json(json);
            DEBUG_MSG("[MessageTextRepository::read] Read message with ID: " + std::to_string(id));
            return message;
        } else {
            DEBUG_MSG("[MessageTextRepository::read] Message not found with ID: " + std::to_string(id));
            return std::nullopt;
        }
    } catch (const std::exception& e) {
        ERROR_MSG("[MessageTextRepository::read] Error: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool MessageTextRepository::update(const MessageText& message) {
    try {
        int id = message.get_id();
        std::string key = "message:" + std::to_string(id);
        
        auto existing = redis->get(key);
        if (!existing) {
            DEBUG_MSG("[MessageTextRepository::update] Message not found with ID: " + std::to_string(id));
            return false;
        }
        
        auto json = message.to_json();
        redis->set(key, json.dump());
        DEBUG_MSG("[MessageTextRepository::update] Updated message with ID: " + std::to_string(id));
        return true;
    } catch (const std::exception& e) {
        ERROR_MSG("[MessageTextRepository::update] Error: " + std::string(e.what()));
        return false;
    }
}

bool MessageTextRepository::remove(int id) {
    try {
        std::string key = "message:" + std::to_string(id);
        long long removed = redis->del(key);
        
        if (removed > 0) {
            DEBUG_MSG("[MessageTextRepository::remove] Removed message with ID: " + std::to_string(id));
            return true;
        } else {
            DEBUG_MSG("[MessageTextRepository::remove] Message not found with ID: " + std::to_string(id));
            return false;
        }
    } catch (const std::exception& e) {
        ERROR_MSG("[MessageTextRepository::remove] Error: " + std::string(e.what()));
        return false;
    }
}

int MessageTextRepository::get_next_id() {
    return redis->incr("message_id_counter");
}