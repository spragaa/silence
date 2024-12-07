#pragma once

#include "common.hpp"

#include <string>
#include <set>
#include <optional>
#include <nlohmann/json.hpp>

namespace common {

class Chat {
public:
	Chat() = default;
	Chat(
		const int& id,
		const int& user1_id,
		const int& user2_id
	);
    
	bool is_deleted() const noexcept;
	int get_id() const noexcept;
	int get_user1_id() const noexcept;
	int get_user2_id() const noexcept;
	Timestamp get_created_timestamp() const noexcept;
	std::optional<Timestamp>get_deleted_timestamp() const noexcept;
	std::optional<Timestamp>get_last_edited_timestamp() const noexcept;
    
	void set_id(const int& id) noexcept;
	void set_user1_id(const int& user1_id) noexcept;
	void set_user2_id(const int& user2_id) noexcept;
	void set_created_timestamp(const Timestamp& timestamp) noexcept;
	void set_last_edited_timestamp(const Timestamp& timestamp) noexcept;
	void set_deleted_timestamp(const Timestamp& timestamp) noexcept;
	void set_deleted(const bool& deleted) noexcept;
    
	nlohmann::json to_json() const;
	Chat from_json(const nlohmann::json& j);
    
private:
	int _id;
	int _user1_id;
	int _user2_id;
	bool _deleted;
	std::optional<Timestamp> _deleted_timestamp;
	Timestamp _created_timestamp;
	std::optional<Timestamp> _last_edited_timestamp;
};


} // namespace common