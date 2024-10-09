#include "message_text.hpp"

MessageText::MessageText(
	const std::string& text
	) : id(0),
	text(text)
{
}

int MessageText::get_id() const noexcept {
	return id;
}

std::string MessageText::get_text() const noexcept {
	return text;
}

void MessageText::set_id(const int& msg_id) noexcept {
	id = msg_id;
}

void MessageText::set_text(const std::string& t) noexcept {
	text = t;
}

nlohmann::json MessageText::to_json() const {
	nlohmann::json j;
	j["id"] = id;
	j["text"] = text;
	return j;
}

MessageText MessageText::from_json(const nlohmann::json& j) {
	MessageText msg(j["text"]);
	msg.id = j["id"];
	msg.text = j["text"];
	return msg;
}