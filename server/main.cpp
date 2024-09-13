#include <iostream>

#include "server.hpp"

int main() {
    Server server(55555, 128);
    server.start();

    return 0;
}