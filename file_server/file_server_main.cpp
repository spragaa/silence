#include "file_server.hpp"

int main() {
	file_server::FileServer server(
	    55544, 
		16 
	);

	server.start();
	
	return 0;
}