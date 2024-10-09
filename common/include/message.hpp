#pragma once

#include "common.hpp"
#include "message_metadata.hpp"
#include "message_text.hpp"

class Message {
public:

	Message(const MessageMetadata& msg_metadata, const MessageText& msg_text);

	nlohmann::json to_json() const;
	Message from_json(const nlohmann::json& j);

private:
    MessageMetadata msg_metadata;
    MessageText msg_text;
};
