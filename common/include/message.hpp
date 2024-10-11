#pragma once

#include "common.hpp"
#include "message_metadata.hpp"
#include "message_text.hpp"

class Message {
public:

	Message(const int& sender_id, const int& receiver_id, const std::string& text);

	nlohmann::json to_json() const;
	Message from_json(const nlohmann::json& j);
	int get_id() const;
private:
    static int message_id_counter;
    MessageMetadata msg_metadata;
	MessageText msg_text;
};
