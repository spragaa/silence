#pragma once

#include "common.hpp"

#include <string>
#include <set>
#include <optional>
#include <nlohmann/json.hpp>

namespace common {

class MessageText {
public:
	MessageText() = default;
	MessageText(const int& id, const std::string& text);

	int get_id() const noexcept;
	std::string get_text() const noexcept;

	void set_id(const int& msg_id) noexcept;
	void set_text(const std::string& t) noexcept;

	nlohmann::json to_json() const;
	MessageText from_json(const nlohmann::json& j);

private:
	int _id;
	std::string _text;
};

}