#include <iostream>

#include "server.hpp"

int main() {
	const unsigned short port = 55555;
	const unsigned int thread_pool_size = 128;
	// read them from config
	const std::string user_metadata_db_connection_string = "host=localhost port=5432 dbname=user_metadata user=postgres password=pass";
	const std::string msg_text_db_connection_string = "host=localhost port=5432 dbname=message_metadata user=postgres password=pass";
	const std::string message_text_db_connection_string = "redis://spraga@127.0.0.1:6379"; // simplify? - no need, because we will read it from json

	Server server(port,
	              thread_pool_size,
	              user_metadata_db_connection_string,
	              msg_text_db_connection_string,
	              message_text_db_connection_string);
	server.start();

	return 0;
}