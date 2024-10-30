#include "client.hpp"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
	// this is only needed for local testing
	// in prod there should be one file with metadata,
	// so user will provide his nickname only on first launch / registration,
	// after that, on each launch programm should read that file
	// -> no need to ask for user's nickname
	std::string nickname;
	std::cout << "Enter nickname: ";
	std::getline(std::cin, nickname);


	client::Client client("127.0.0.1", 55555, nickname);
	client.run();

	return 0;
}