#include "file_server.hpp"

int main() {
	FileServer server(55544, 16, std::string(SOURCE_DIR) + "/file_server/media_file_system/", 1024 * 1024 * 1024);
	server.init();
	server.start();
	return 0;
}