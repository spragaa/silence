#include "message_text_repository.hpp"
#include <regex>

MessageTextRepository::MessageTextRepository(const std::string& connection_string) {
    try {
        DEBUG_MSG("[MessageTextRepository::MessageTextRepository] Parsing message_text database connection string: " + connection_string);
        auto connection_options = parse_config_string(connection_string);
        
        // debug - don't log password in production!
        // i will...
        DEBUG_MSG("[MessageTextRepository::MessageTextRepository] Parsed options - Host: " + connection_options.host + 
                  ", Port: " + std::to_string(connection_options.port) + 
                  ", User: " + connection_options.user +
                  ", Password: " + connection_options.password + 
                  ", DB: " + std::to_string(connection_options.db));
        
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

int MessageTextRepository::create(const MessageText& msg) {
    try {
        std::string key = std::to_string(msg.get_id());
        std::string value = msg.get_text();
        DEBUG_MSG("[MessageTextRepository::create] Attempting to set key: " + key + " with value: " + value);
        redis->set(key, value);
        
        auto result = redis->get(key);
        if (!result) {
            ERROR_MSG("[MessageTextRepository::create] Verification failed. Unable to retrieve set value");
            return 0;
        }
    } catch(const sw::redis::Error& e) {
        ERROR_MSG("[MessageTextRepository::create] Redis error: " + std::string(e.what()));
        return 0;
    } catch(const std::exception& e) {
        ERROR_MSG("[MessageTextRepository::create] " + std::string(e.what()));
        return 0;
    }

    DEBUG_MSG("[MessageTextRepository::create] MessageText created successfully: " + msg.get_text() + " with id: " + std::to_string(msg.get_id()));
    return msg.get_id();
}

std::optional<MessageText> MessageTextRepository::read(int id) {
    try {
        auto result = redis->get(std::to_string(id));
        if (result) {
            DEBUG_MSG("[MessageTextRepository::read] Successfully read message with id: " + std::to_string(id));
            return MessageText(id, *result);
        } else {
            DEBUG_MSG("[MessageTextRepository::read] No message found with id: " + std::to_string(id));
            return std::nullopt;
        }
    } catch (const sw::redis::Error& e) {
        ERROR_MSG("[MessageTextRepository::read] Redis error: " + std::string(e.what()));
        return std::nullopt;
    } catch (const std::exception& e) {
        ERROR_MSG("[MessageTextRepository::read] " + std::string(e.what()));
        return std::nullopt;
    }
}

bool MessageTextRepository::update(const MessageText& message) {
    try {
        auto key = std::to_string(message.get_id());
        auto exists = redis->exists(key);
        if (exists) {
            redis->set(key, message.get_text());
            DEBUG_MSG("[MessageTextRepository::update] Successfully updated message with id: " + key);
            return true;
        } else {
            DEBUG_MSG("[MessageTextRepository::update] No message found with id: " + key);
            return false;
        }
    } catch (const sw::redis::Error& e) {
        ERROR_MSG("[MessageTextRepository::update] Redis error: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        ERROR_MSG("[MessageTextRepository::update] " + std::string(e.what()));
        return false;
    }
}

bool MessageTextRepository::remove(int id) {
    try {
        auto key = std::to_string(id);
        auto removed = redis->del(key);
        if (removed > 0) {
            DEBUG_MSG("[MessageTextRepository::remove] Successfully removed message with id: " + key);
            return true;
        } else {
            DEBUG_MSG("[MessageTextRepository::remove] No message found with id: " + key);
            return false;
        }
    } catch (const sw::redis::Error& e) {
        ERROR_MSG("[MessageTextRepository::remove] Redis error: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        ERROR_MSG("[MessageTextRepository::remove] " + std::string(e.what()));
        return false;
    }
}