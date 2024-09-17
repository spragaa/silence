#include <iostream>

#include "server.hpp"

int main() {
	Server server(55555, 8);
	server.start();

	return 0;
}