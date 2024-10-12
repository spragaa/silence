#include "message.hpp"

int Message::message_id_counter = 1;

Message::Message(const int& sender_id, const int& receiver_id, const std::string& text) :    
	metadata(message_id_counter, sender_id, receiver_id), text(message_id_counter++, text)
{
}

nlohmann::json Message::to_json() const {
	nlohmann::json j;
	j.update(metadata.to_json());
	j.update(text.to_json());

	return j;
}

int Message::get_id() const {
    return metadata.get_id();
}

MessageText get_text() const {
    return text;
}

MessageMetadata get_metadata() const {
    return metadata;
}