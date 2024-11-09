#pragma once

#include <string>

namespace server {

struct ServerConfig {
	unsigned short _port;
	unsigned int _thread_pool_size;
	std::string _user_metadata_db_connection_string;
	std::string _msg_metadata_db_connection_string;
	std::string _msg_text_db_connection_string;
	std::string _file_server_host;
	std::string _file_server_port;
};

}