#include "message.hpp"

int Message::message_id_counter = 0;

Message::Message(
	const int sender_id,
	const int receiver_id,
	const std::string& text

	// const std::optional<Media> & media
	) : id(message_id_counter++),
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
