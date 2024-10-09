#include "message_metadata.hpp"

MessageMetadata::MessageMetadata(
	const int sender_id,
	const int receiver_id
	) : id(0),
	sender_id(sender_id),
	receiver_id(receiver_id),
	deleted(false),
	created_timestamp(std::chrono::system_clock::now())
{
}

int MessageMetadata::get_id() const noexcept {
	return id;
}

int MessageMetadata::get_sender_id() const noexcept {
	return sender_id;
}

int MessageMetadata::get_receiver_id() const noexcept {
	return receiver_id;
}

bool MessageMetadata::is_deleted() const noexcept {
	return deleted;
}

Timestamp MessageMetadata::get_created_timestamp() const noexcept {
	return created_timestamp;
}

std::optional<Timestamp>MessageMetadata::get_deleted_timestamp() const noexcept {
	if (deleted_timestamp != std::nullopt) {
		return deleted_timestamp;
	}

	return std::nullopt;
}

std::optional<Timestamp>MessageMetadata::get_last_edited_timestamp() const noexcept {
	if (last_edited_timestamp != std::nullopt) {
		return last_edited_timestamp;
	}

	return std::nullopt;
}

void MessageMetadata::set_id(const int& msg_id) noexcept {
	id = msg_id;
}

void MessageMetadata::set_sender(const int& s) noexcept {
	sender_id = s;
}

void MessageMetadata::set_created_timestamp(const Timestamp& timestamp) noexcept {
	created_timestamp = timestamp;
}

void MessageMetadata::set_last_edited_timestamp(const Timestamp& timestamp) noexcept {
	last_edited_timestamp = timestamp;
}

void MessageMetadata::set_deleted_timestamp(const Timestamp& timestamp) noexcept {
	deleted_timestamp = timestamp;
}

void MessageMetadata::set_deleted(const bool& d) noexcept {
	deleted = d;
}

nlohmann::json MessageMetadata::to_json() const {
	nlohmann::json j;
	j["id"] = id;
	j["sender_id"] = sender_id;
	j["receiver_id"] = receiver_id;
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

MessageMetadata MessageMetadata::from_json(const nlohmann::json& j) {
	MessageMetadata msg(j["sender_id"], j["receiver_id"]);
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