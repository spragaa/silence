#include "message.hpp"

int Message::message_id_counter = 1;

Message::Message(const int& sender_id, const int& receiver_id, const std::string& text) :    
	msg_metadata(message_id_counter, sender_id, receiver_id), msg_text(message_id_counter++, text)
{
}

nlohmann::json Message::to_json() const {
	nlohmann::json j;
	j.update(msg_metadata.to_json());
	j.update(msg_text.to_json());

	return j;
}

int Message::get_id() const {
    return msg_metadata.get_id();
}