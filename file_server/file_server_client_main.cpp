#include "file_server_client.hpp"

int main() {
	file_server::FileServerClient client(
		"localhost",
		"55555"
		);

	return 0;
}