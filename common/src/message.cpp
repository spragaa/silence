#include "message.hpp"

Message::Message(const int& sender_id, const int& receiver_id, const std::string& text) :
	metadata(0, sender_id, receiver_id), text(0, text)
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

MessageText Message::get_text() const {
	return text;
}

MessageMetadata Message::get_metadata() const {
	return metadata;
}

void Message::set_id(const int& id) {
    metadata.set_id(id);
    text.set_id(id);
}
