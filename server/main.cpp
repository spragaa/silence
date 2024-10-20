// #include <iostream>

// #include "server.hpp"

// int main() {
// 	const unsigned short port = 55555;
// 	const unsigned int thread_pool_size = 128;
// 	// read them from config
// 	const std::string user_metadata_db_connection_string = "host=localhost port=5432 dbname=user_metadata user=postgres password=pass";
// 	const std::string msg_text_db_connection_string = "host=localhost port=5432 dbname=message_metadata user=postgres password=pass";
// 	const std::string message_text_db_connection_string = "redis://spraga@127.0.0.1:6379"; // simplify? - no need, because we will read it from json

// 	Server server(port,
// 	              thread_pool_size,
// 	              user_metadata_db_connection_string,
// 	              msg_text_db_connection_string,
// 	              message_text_db_connection_string);
// 	server.start();

// 	return 0;
// }
// 
#include "file_server_client.hpp"
#include <iostream>

int main() {
    try {
        FileServerClient client("localhost", "55544");

        std::cout << "Listing files:\n" << client.list_files() << std::endl;
        std::cout << "Uploading file:\n" << client.upload_file("test.txt", "Hello, World!") << std::endl;
        std::cout << "Downloading file:\n" << client.download_file("test.txt") << std::endl;
        std::cout << "Deleting file:\n" << client.delete_file("test.txt") << std::endl;
        std::cout << "Listing files after deletion:\n" << client.list_files() << std::endl;
    }
    catch(std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}