#include "message.hpp"

namespace common {

Message::Message(const int& sender_id, const int& receiver_id, const std::string& text) :
	_metadata(0, sender_id, receiver_id, 0), _text(0, text)
{
}

nlohmann::json Message::to_json() const {
	nlohmann::json j;
	j.update(_metadata.to_json());
	j.update(_text.to_json());

	return j;
}

int Message::get_id() const {
	return _metadata.get_id();
}

MessageText Message::get_text() const {
	return _text;
}

MessageMetadata Message::get_metadata() const {
	return _metadata;
}

void Message::set_id(const int& id) {
	_metadata.set_id(id);
	_text.set_id(id);
}

}