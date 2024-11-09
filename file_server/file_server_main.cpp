#include "file_server.hpp"

int main() {
	file_server::FileServer server(
	    9080, 
		16, 
		std::string(SOURCE_DIR) + "/file_server/media_file_system/", 
		1024 * 1024 * 10 // 10mb
	);

	server.start();
	
	return 0;
}