#include <iostream>

#include "server.hpp"

int main() {
	Server server(55555, 8);
	server.start();

	return 0;
}

// A sends a message to B
// on server we store a message from A to B with all needed metadata
// client has its own copy of the message stored, 
// lets say that client can store only last N messages
// to open chat, it make request to the server to get last M messages
// to do so, server find last M messages where receiver or/and sender is A and B in the messages table
// 
// on the client side I would like to have a 