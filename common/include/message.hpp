#pragma once

#include "common.hpp"
#include "message_metadata.hpp"
#include "message_text.hpp"

class Message {
public:

    Message() = default;
	Message(const int& sender_id, const int& receiver_id, const std::string& text);

	nlohmann::json to_json() const;
	Message from_json(const nlohmann::json& j);
	int get_id() const;
	MessageText get_text() const;
	MessageMetadata get_metadata() const;
	
private:
    static int message_id_counter;
    MessageMetadata metadata;
	MessageText text;
};
