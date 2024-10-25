#include "message_text.hpp"

MessageText::MessageText(const int& id, const std::string& text) :
	_id(id), _text(text)
{
}

int MessageText::get_id() const noexcept {
	return _id;
}

std::string MessageText::get_text() const noexcept {
	return _text;
}

void MessageText::set_id(const int& msg_id) noexcept {
	_id = msg_id;
}

void MessageText::set_text(const std::string& t) noexcept {
	_text = t;
}

nlohmann::json MessageText::to_json() const {
	nlohmann::json j;
	j["id"] = _id;
	j["text"] = _text;
	return j;
}

MessageText MessageText::from_json(const nlohmann::json& j) {
	MessageText msg(j["id"], j["text"]);
	msg._id = j["id"];
	msg._text = j["text"];
	return msg;
}