#include <iostream>

#include "server.hpp"

int main() {	
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