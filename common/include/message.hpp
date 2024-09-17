#include "common.hpp"

#include <string>
#include <set>
#include <optional>
#include <nlohmann/json.hpp>

// class Media;

class Message {
public:

    // Message();
	Message(
		const int sender_id,
		const int receiver_id,
		const std::string& text

		// const std::optional<Media> & media
		);
	
	bool                    is_deleted() const noexcept;
	int                     get_id() const noexcept;
	int                     get_sender_id() const noexcept;
	int                     get_receiver_id() const noexcept;
	std::string             get_text() const noexcept;
	Timestamp               get_created_timestamp() const noexcept;
	std::optional<Timestamp>get_deleted_timestamp() const noexcept;
	std::optional<Timestamp>get_last_edited_timestamp() const noexcept;
	
	// void set_id(const int& msg_id) noexcept;
	// void set_text(const std::string& t) noexcept;
	// void set_sender(const std::string& t) noexcept;
	// bool operator < (const Message& other) const;

    nlohmann::json to_json() const;
    static Message from_json(const nlohmann::json& j);
	
private:

	static int message_id_counter;
	int id;
	int sender_id;
	int receiver_id;
	std::string reveiver_nickname;
	std::string text;
	bool deleted;

	// std::optional<Media> media;

	Timestamp created_timestamp;
	std::optional<Timestamp>deleted_timestamp;
	std::optional<Timestamp>last_edited_timestamp;
};
