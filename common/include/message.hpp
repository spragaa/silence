#include "common.hpp"

#include <string>
#include <optional>

// class Media;

class Message {
public:

	Message(
		const int sender_id,
		const int receiver_id,
		const std::string& text

		// const std::optional<Media> & media
		);

	int                     get_id() const noexcept;
	int                     get_sender_id() const noexcept;

	int                     get_receiver_id() const noexcept;
	std::string             get_text() const noexcept;
	bool                    is_deleted() const noexcept;
	Timestamp               get_created_timestamp() const noexcept;
	std::optional<Timestamp>get_deleted_timestamp() const noexcept;
	std::optional<Timestamp>get_last_edited_timestamp() const noexcept;

private:

	static int message_id_counter;
	int id;
	int sender_id;
	int receiver_id;

	std::string text;
	bool deleted;

	// std::optional<Media> media;

	Timestamp created_timestamp;
	std::optional<Timestamp>deleted_timestamp;
	std::optional<Timestamp>last_edited_timestamp;
};
