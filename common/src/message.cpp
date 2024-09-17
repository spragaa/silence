#include "message.hpp"

int Message::message_id_counter = 1;

// Message::Message() :
//     id(0),
//     sender_id(0),
//     receiver_id(0),
//     text(""),
//     deleted(false),
//     created_timestamp(std::chrono::system_clock::now())
// {}

Message::Message(
	const int sender_id,
	const int receiver_id,
	const std::string& text

	// const std::optional<Media> & media
	) : id(++message_id_counter),
	sender_id(sender_id),
	receiver_id(receiver_id),
	text(text),
	deleted(false),
	created_timestamp(std::chrono::system_clock::now())
{
}

int Message::get_id() const noexcept {
	return id;
}

int Message::get_sender_id() const noexcept {
	return sender_id;
}

int Message::get_receiver_id() const noexcept {
	return receiver_id;
}

std::string Message::get_text() const noexcept {
	return text;
}

bool Message::is_deleted() const noexcept {
	return deleted;
}

Timestamp Message::get_created_timestamp() const noexcept {
	return created_timestamp;
}

std::optional<Timestamp>Message::get_deleted_timestamp() const noexcept {
	if (deleted_timestamp != std::nullopt) {
		return deleted_timestamp;
	}

	return std::nullopt;
}

std::optional<Timestamp>Message::get_last_edited_timestamp() const noexcept {
	if (last_edited_timestamp != std::nullopt) {
		return last_edited_timestamp;
	}

	return std::nullopt;
}

// void Message::set_id(const int& msg_id) noexcept {
//     id = msg_id;
// }

// bool Message::operator<(const Message& other) const {
//     return id < other.id;
// }

nlohmann::json Message::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["sender_id"] = sender_id;
    j["receiver_id"] = receiver_id;
    j["text"] = text;
    j["deleted"] = deleted;
    j["created_timestamp"] = created_timestamp.time_since_epoch().count();
    if (deleted_timestamp) {
        j["deleted_timestamp"] = deleted_timestamp->time_since_epoch().count();
    }
    if (last_edited_timestamp) {
        j["last_edited_timestamp"] = last_edited_timestamp->time_since_epoch().count();
    }
    return j;
}

Message Message::from_json(const nlohmann::json& j) {
    Message msg(j["sender_id"], j["receiver_id"], j["text"]);
    msg.id = j["id"];
    msg.deleted = j["deleted"];
    msg.created_timestamp = Timestamp(std::chrono::nanoseconds(j["created_timestamp"]));
    if (j.contains("deleted_timestamp")) {
        msg.deleted_timestamp = Timestamp(std::chrono::nanoseconds(j["deleted_timestamp"]));
    }
    if (j.contains("last_edited_timestamp")) {
        msg.last_edited_timestamp = Timestamp(std::chrono::nanoseconds(j["last_edited_timestamp"]));
    }
    return msg;
}