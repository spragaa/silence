#include "file_server.hpp"

int main() {
	std::cout << "trying to create server in main" << std::endl;
    
	
	FileServer server(
	    9080, 
		16 
	);
	
	// server.stop();
	// server.start();
	
	return 0;
}