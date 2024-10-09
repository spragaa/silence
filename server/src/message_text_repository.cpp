#include "message_text_repository.hpp"
#include <regex>

MessageTextRepository::MessageTextRepository(const std::string& connection_string) {
	DEBUG_MSG("[MessageTextRepository::MessageTextRepository] Parsing message_text database connection string" + connection_string);
	auto connection_options = parseConnectionString(connection_string);
	redis = std::make_unique<sw::redis::Redis>(connection_options);
	INFO_MSG("[MessageTextRepository::MessageTextRepository] Successfully connected to messate_text Redis database");
}

sw::redis::ConnectionOptions MessageTextRepository::parseConnectionString(const std::string& connection_string) {
	sw::redis::ConnectionOptions options;

	std::regex connection_regex("redis://((?:\\w+):(?:\\w+)@)?([\\w.]+):(\\d+)(?:/(.+))?");
	std::smatch matches;

	if (std::regex_match(connection_string, matches, connection_regex)) {
		if (matches[1].matched) {
			std::string auth = matches[1].str();
			size_t colon_pos = auth.find(':');
			if (colon_pos != std::string::npos) {
				options.user = auth.substr(0, colon_pos);
				options.password = auth.substr(colon_pos + 1, auth.length() - colon_pos - 2); // -2 to remove the '@'
			}
		}
		options.host = matches[2].str();
		options.port = std::stoi(matches[3].str());
		if (matches[4].matched) {
			options.db = std::stoi(matches[4].str());
		}
	} else {
		FATAL_MSG("[MessageTextRepository::parseConnectionString] Invalid connection string format");
	}

	DEBUG_MSG("[MessageTextRepository::parseConnectionString] Parsed succesfully!");

	return options;
}