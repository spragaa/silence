#pragma once

#include "debug.hpp"
#include "message_text.hpp"
#include <redis++.h>

#include <memory>
#include <string>

namespace server {

class MessageTextRepository {
public:
	MessageTextRepository(const std::string& connection_string);

	int create(const common::MessageText& message);
	std::optional<common::MessageText> read(int id);
	bool update(const common::MessageText& message);
	bool remove(int id);

private:
	sw::redis::ConnectionOptions parse_config_string(const std::string& connection_string);
	int get_next_id();

private:
	std::unique_ptr<sw::redis::Redis> _redis;
};

}