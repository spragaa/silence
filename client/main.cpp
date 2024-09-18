#include "client.hpp"

int main(int argc, char *argv[]) {
    std::string arg_nickname;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg.rfind("--nickname=", 0) == 0) {
            arg_nickname = arg.substr(11);
        }
    }
    
    Client client("127.0.0.1", 55555, arg_nickname);
    client.run();

    return 0;
}
