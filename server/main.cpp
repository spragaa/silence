#include <iostream>

#include "server.hpp"

int main() {
	const unsigned short port = 55555;
	const unsigned int thread_pool_size = 128;
	// read them from config
	const std::string user_metadata_db_connection_string = "host=localhost port=5432 dbname=user_metadata user=postgres password=pass";
	const std::string msg_text_db_connection_string = "host=localhost port=5432 dbname=message_metadata user=postgres password=pass";
	const std::string message_text_db_connection_string = "redis://spraga@127.0.0.1:6379";
	const std::string file_server_host = "localhost";
	const std::string file_server_port = "55544";
	Server server(port,
	              thread_pool_size,
	              user_metadata_db_connection_string,
	              msg_text_db_connection_string,
	              message_text_db_connection_string,
	              file_server_host,
	              file_server_port);
	server.start();

	return 0;
}

// #include "file_server_client.hpp"
// #include <iostream>
// namespace fs = std::filesystem;

// int main() {
//     try {
//         FileServerClient client("localhost", "55544");
//         std::string filename = "logscr.txt";
//         std::string filepath = "/home/logi/Downloads";

//         fs::path full_path = fs::path(filepath) / filename;
//         if (!fs::exists(full_path)) {
//             std::cerr << "Error: File does not exist: " << full_path << std::endl;
//             return EXIT_FAILURE;
//         }

//         std::cout << "Uploading file: " << full_path << std::endl;
//         std::string upload_result = client.upload_file(filename, filepath);
//         std::cout << "Upload result: " << upload_result << std::endl;

//         // std::cout << "Listing files:\n" << client.list_files() << std::endl;
//     }
//     catch(std::exception const& e) {
//         std::cerr << "Error in main: " << e.what() << std::endl;
//         return EXIT_FAILURE;
//     }
//     return EXIT_SUCCESS;
// }