#include "message.hpp"

// idea of using id here is stupid, or implemented incorrectly
// for example, on each run message_id_counter = 1, but messsage db isn't reset
int Message::message_id_counter = 1;

// mutex is probably needed, but it will create a bottleneck
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

MessageText Message::get_text() const {
	return text;
}

MessageMetadata Message::get_metadata() const {
	return metadata;
}