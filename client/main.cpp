#include "client.hpp"

int main() {
    Client client("127.0.0.1", 55555);
    client.run();
 
    return 0;
}