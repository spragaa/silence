#include "client.hpp"

Client::Client(const std::string& server_address,
               unsigned short server_port,
               const std::string& nickname)
	: io_service()
	, socket(io_service)
	, server_address(server_address)
	, server_port(server_port)
	, user(nickname, "")
{
}

User Client::get_user() {
	return user;
}

void inline Client::show_actions() {
    // std::string created_nickname;
    // if (user.get_nickname().size() != 0) {
    //     std::cout << user.get_nickname() << ", please choose the action type from the list:\n";
    // } else {
    //     std::cout << "To be able to use full functionality of this chat application, you should register first!" << std::endl;
    //     std::cout << "Create a nickname to your account:";
    //     std::getline(std::cin, nickname);
    //     std::cout << std::endl;
    // }

    std::cout << user.get_nickname() << ", please select the number of the action you would like to perform now from the list below:" << std::endl;
    std::cout << "0. Register new user" << std::endl;
    std::cout << "1. Authorize user" << std::endl;
    std::cout << "2. Get list of chats" << std::endl;
    std::cout << "3. Send message" << std::endl;
    std::cout << "4. Get offline messages" << std::endl;
    std::cout << "5. KYS" << std::endl;
}

void Client::run() {
	try {
		boost::asio::ip::tcp::resolver resolver(io_service);
		boost::asio::ip::tcp::resolver::query query(server_address,
		                                            std::to_string(server_port));
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
			resolver.resolve(query);
		boost::asio::connect(socket, endpoint_iterator);

        while (true) {
            int action_type = -1;
            show_actions();
            
            while(action_type < 0 || action_type >= 6) {
                std::cin >> action_type;
                if (action_type < 0 || action_type >= 6) {
                    std::cout << "Incorrect action type has been chosen:" << action_type << std::endl;
                }
            }
            
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            switch (action_type) {
                case 0: {
                    std::string nickname, password;
                    std::cout << "Enter nickname: ";
                    std::getline(std::cin, nickname);

                    std::cout << "Enter password: ";
                    std::getline(std::cin, password);

                    // this should be outside of the case {}
                    nlohmann::json request;
                    request["type"] = "register";
                    request["nickname"] = nickname;
                    request["password"] = password;
                    
                    // this should be in a separate func or at least out side of the case {}
                    boost::asio::write(socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
                    nlohmann::json response = receive_response();
                    std::cout << "Server response: " << response.dump() << std::endl;
                    break;                
                }
                case 1: {
                    std::string nickname, password;
                    std::cout << "Enter nickname: ";
                    std::getline(std::cin, nickname);

                    std::cout << "Enter password: ";
                    std::getline(std::cin, password);

                    // this should be outside of the case {}
                    nlohmann::json request;
                    request["type"] = "authorize";
                    request["nickname"] = nickname;
                    request["password"] = password;
                    
                    // this should be in a separate func or at least out side of the case {}
                    boost::asio::write(socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
                    nlohmann::json response = receive_response();
                    std::cout << "Server response: " << response.dump() << std::endl;
                    break;                
                }
                case 2: {
                    std::cout << "TODO: " << std::endl;
                    std::cout << "Getting list of all chats... " << std::endl;
                    std::cout << std::endl;
                    break;
                }
                case 3: {
                    std::string receiver_nickname;
                    std::cout << "Provide receiver's nickname: " << std::endl;
                    std::getline(std::cin, receiver_nickname);
                    std::cout << std::endl;

                    std::string message;
                    std::cout << "Enter the message to send to " << receiver_nickname << ": ";
                    std::getline(std::cin, message);
                    std::cout << "Sending message to " << receiver_nickname << ": " << message << std::endl;
                    break;
                }
                case 4: {
                    std::cout << "TODO" << std::endl;
                    std::cout << "Getting offline messages... " << std::endl;
                    std::cout << std::endl;
                    break;
                }
                case 5: {
                    std::cout << "KYS KYS KYS KYS KYS KYS KYS ";
                    std::cout << std::endl;
                    break;
                }
                default: {
                    std::cout << "Invalid option." << std::endl;
                    break;
                }
            }

            // std::string command = read_user_text();

            // if (command == "exit") {
            //     break;
            // }

            // std::istringstream command_stream(command);
            // std::string action;
            // std::getline(command_stream, action, ':');
            // std::string data = command.substr(action.length() + 1);

            // nlohmann::json request;
            // request["type"] = action;
            // request["data"] = data;

            // boost::asio::write(socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
            // nlohmann::json response = receive_response();
            // std::cout << "Server response: " << response.dump() << std::endl;
		}

		socket.close();
	} catch (const std::exception& e) {
		std::cerr << "Exception in Client::run() : " << e.what() << std::endl;
	}
}

std::string Client::read_user_text() const noexcept{
	std::string message;
	std::getline(std::cin, message);

	return message;
}

void Client::send_message(const std::string& message) {
	DEBUG_MSG("send_message" + get_socket_info(socket));
	boost::asio::write(socket, boost::asio::buffer(message + "\r\n\r\n"));
}

std::string Client::receive_response() {
	boost::asio::streambuf response_buf;
	boost::system::error_code error;

	boost::asio::read_until(socket, response_buf, "\r\n\r\n", error);

	if (error && (error != boost::asio::error::eof)) {
		throw std::runtime_error(
				  "Error while receiveing response: " + error.message());
	}

	std::istream response_stream(&response_buf);
	std::string response;

	std::getline(response_stream, response);

	return response;
}
