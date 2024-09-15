#include "client.hpp"

int main(int argc, char *argv[]) {
    std::string nickname;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg.rfind("--nickname=", 0) == 0) {
            nickname = arg.substr(11);
        }
    }

    if (!nickname.empty()) {
        std::cout << "Nickname: " << nickname << std::endl;
    } else {
        std::cout << "No nickname provided." << std::endl;
        nickname = "noname";
    }

    Client client("127.0.0.1", 55555, nickname);

    client.run();

    return 0;
}
