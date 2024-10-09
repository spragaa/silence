#pragma once

#include "debug.hpp"
#include "message_text.hpp"
// #include "/home/logi/myself/programming/cpp/chat_application/build/_deps/redis_plus_plus-src/src/sw/redis++/redis++.h""
#include <sw/redis++/redis++.h>

#include <memory>
#include <string>

class MessageTextRepository {
public:
    MessageTextRepository(const std::string& connection_string);

    int create(const MessageText& message);
    std::optional<MessageText> read(int id);
    bool update(const MessageText& message);
    bool remove(int id);

private:
    sw::redis::ConnectionOptions parseConnectionString(const std::string& connection_string);
    int get_next_id();

private:
    std::unique_ptr<sw::redis::Redis> redis;
};