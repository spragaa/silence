#include "chat.hpp"

namespace common {

Chat::Chat(
	const int& id,
	const int& user1_id,
	const int& user2_id
) : _id(id),
	_user1_id(user1_id),
	_user2_id(user2_id),
	_deleted(false),
	_created_timestamp(std::chrono::system_clock::now())
{
}

int Chat::get_id() const noexcept {
	return _id;
}

int Chat::get_user1_id() const noexcept {
	return _user1_id;
}

int Chat::get_user2_id() const noexcept {
	return _user2_id;
}

bool Chat::is_deleted() const noexcept {
	return _deleted;
}

Timestamp Chat::get_created_timestamp() const noexcept {
	return _created_timestamp;
}

std::optional<Timestamp>Chat::get_deleted_timestamp() const noexcept {
	if (_deleted_timestamp != std::nullopt) {
		return _deleted_timestamp;
	}

	return std::nullopt;
}

std::optional<Timestamp>Chat::get_last_edited_timestamp() const noexcept {
	if (_last_edited_timestamp != std::nullopt) {
		return _last_edited_timestamp;
	}

	return std::nullopt;
}

void Chat::set_id(const int& id) noexcept {
	_id = id;
}

void Chat::set_user1_id(const int& user1_id) noexcept {
	_user1_id = user1_id;
}

void Chat::set_user2_id(const int& user2_id) noexcept {
	_user2_id = user2_id;
}

void Chat::set_created_timestamp(const Timestamp& timestamp) noexcept {
	_created_timestamp = timestamp;
}

void Chat::set_last_edited_timestamp(const Timestamp& timestamp) noexcept {
	_last_edited_timestamp = timestamp;
}

void Chat::set_deleted_timestamp(const Timestamp& timestamp) noexcept {
	_deleted_timestamp = timestamp;
}

void Chat::set_deleted(const bool& d) noexcept {
	_deleted = d;
}

nlohmann::json Chat::to_json() const {
	nlohmann::json j;
	j["id"] = _id;
	j["user1_id"] = _user1_id;
	j["user2_id"] = _user2_id;
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

Chat Chat::from_json(const nlohmann::json& j) {
	Chat chat(j["id"], j["user1_id"], j["user2_id"]);
	chat._id = j["id"];
	chat._deleted = j["deleted"];
	chat._created_timestamp = Timestamp(std::chrono::nanoseconds(j["created_timestamp"]));
	if (j.contains("deleted_timestamp")) {
		chat._deleted_timestamp = Timestamp(std::chrono::nanoseconds(j["deleted_timestamp"]));
	}
	if (j.contains("last_edited_timestamp")) {
		chat._last_edited_timestamp = Timestamp(std::chrono::nanoseconds(j["last_edited_timestamp"]));
	}
	return chat;
}

}