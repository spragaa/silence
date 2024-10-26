#include <iostream>

#include "server.hpp"

int main() {
	// const unsigned short port = 55555;
	// const unsigned int thread_pool_size = 128;

	// const std::string user_metadata_db_connection_string = "host=localhost port=5432 dbname=user_metadata user=postgres password=pass";
	// const std::string msg_text_db_connection_string = "host=localhost port=5432 dbname=message_metadata user=postgres password=pass";
	// const std::string message_text_db_connection_string = "redis://spraga@127.0.0.1:6379";
	// const std::string file_server_host = "localhost";
	// const std::string file_server_port = "55544";
	
	ServerConfig config {
	    ._port = 55555,
		._thread_pool_size = 256,
		._user_metadata_db_connection_string = "host=localhost port=5432 dbname=user_metadata user=postgres password=pass",
		._msg_metadata_db_connection_string = "host=localhost port=5432 dbname=message_metadata user=postgres password=pass",
		._msg_text_db_connection_string = "redis://spraga@127.0.0.1:6379",
		._file_server_host = "localhost",
		._file_server_port = "55544"
	};
	
	Server server(config);
	server.start();

	return 0;
}