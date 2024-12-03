#include "message_metadata.hpp"

namespace common {

MessageMetadata::MessageMetadata(
	const int& id,
	const int& sender_id,
	const int& receiver_id,
	const int& chat_id
) : _id(id),
	_sender_id(sender_id),
	_receiver_id(receiver_id),
	_chat_id(chat_id),
	_deleted(false),
	_created_timestamp(std::chrono::system_clock::now())
{
}

int MessageMetadata::get_id() const noexcept {
	return _id;
}

int MessageMetadata::get_sender_id() const noexcept {
	return _sender_id;
}

int MessageMetadata::get_receiver_id() const noexcept {
	return _receiver_id;
}

int MessageMetadata::get_chat_id() const noexcept {
	return _chat_id;
}

bool MessageMetadata::is_deleted() const noexcept {
	return _deleted;
}

Timestamp MessageMetadata::get_created_timestamp() const noexcept {
	return _created_timestamp;
}

std::optional<Timestamp>MessageMetadata::get_deleted_timestamp() const noexcept {
	if (_deleted_timestamp != std::nullopt) {
		return _deleted_timestamp;
	}

	return std::nullopt;
}

std::optional<Timestamp>MessageMetadata::get_last_edited_timestamp() const noexcept {
	if (_last_edited_timestamp != std::nullopt) {
		return _last_edited_timestamp;
	}

	return std::nullopt;
}

void MessageMetadata::set_id(const int& msg_id) noexcept {
	_id = msg_id;
}

void MessageMetadata::set_sender_id(const int& s) noexcept {
	_sender_id = s;
}

void MessageMetadata::set_chat_id(const int& chat_id) noexcept {
	_chat_id = chat_id;
}

void MessageMetadata::set_created_timestamp(const Timestamp& timestamp) noexcept {
	_created_timestamp = timestamp;
}

void MessageMetadata::set_last_edited_timestamp(const Timestamp& timestamp) noexcept {
	_last_edited_timestamp = timestamp;
}

void MessageMetadata::set_deleted_timestamp(const Timestamp& timestamp) noexcept {
	_deleted_timestamp = timestamp;
}

void MessageMetadata::set_deleted(const bool& d) noexcept {
	_deleted = d;
}

nlohmann::json MessageMetadata::to_json() const {
	nlohmann::json j;
	j["id"] = _id;
	j["sender_id"] = _sender_id;
	j["receiver_id"] = _receiver_id;
	j["chat_id"] = _chat_id;
	j["deleted"] = _deleted;
	j["created_timestamp"] = _created_timestamp.time_since_epoch().count();
	if (_deleted_timestamp) {
		j["deleted_timestamp"] = _deleted_timestamp->time_since_epoch().count();
	}
	if (_last_edited_timestamp) {
		j["last_edited_timestamp"] = _last_edited_timestamp->time_since_epoch().count();
	}
	return j;
}

MessageMetadata MessageMetadata::from_json(const nlohmann::json& j) {
	MessageMetadata msg(j["id"], j["sender_id"], j["receiver_id"], j["chat_id"]);
	msg._id = j["id"];
	msg._deleted = j["deleted"];
	msg._created_timestamp = Timestamp(std::chrono::nanoseconds(j["created_timestamp"]));
	if (j.contains("deleted_timestamp")) {
		msg._deleted_timestamp = Timestamp(std::chrono::nanoseconds(j["deleted_timestamp"]));
	}
	if (j.contains("last_edited_timestamp")) {
		msg._last_edited_timestamp = Timestamp(std::chrono::nanoseconds(j["last_edited_timestamp"]));
	}
	return msg;
}

}