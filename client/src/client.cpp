#include "client.hpp"

Client::Client(const std::string& server_address,
               unsigned short server_port)
	: io_service()
	, socket(io_service)
	, server_address(server_address)
	, server_port(server_port)
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
    
    
    // there is always no nickname
    std::cout << user.get_nickname() << ", please select the number of the action you would like to perform now from the list below:" << std::endl;
    std::cout << "0. Register new user" << std::endl;
    std::cout << "1. Authorize user" << std::endl;
    std::cout << "2. Get list of chats" << std::endl;
    std::cout << "3. Send message" << std::endl;
    std::cout << "4. Get offline messages" << std::endl;
    std::cout << "5. KYS" << std::endl;
    
    std::cout << "Input: ";
}

void Client::run() {
	try {
		boost::asio::ip::tcp::resolver resolver(io_service);
		boost::asio::ip::tcp::resolver::query query(server_address,
		                                            std::to_string(server_port));
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
			resolver.resolve(query);
		boost::asio::connect(socket, endpoint_iterator);
		
		// TODO: remembder nickname on the client side
        while (true) {
            int action_type = -1;
            show_actions();
            
            while(action_type < 0 || action_type >= 6) {
                std::cin >> action_type;
                if (action_type < 0 || action_type >= 6) {
                    std::cout << "Incorrect action type has been chosen:" << action_type << std::endl;
                    show_actions();
                }
            }
            
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            nlohmann::json request;
            nlohmann::json response;
            
            switch (action_type) {
                case 0: {
                    std::string nickname, password;
                    std::cout << "Enter nickname: ";
                    std::getline(std::cin, nickname);
                
                    std::cout << "Enter password: ";
                    std::getline(std::cin, password);
                
                    request["type"] = "register";
                    request["nickname"] = nickname;
                    request["password"] = password;
                    
                    boost::asio::write(socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
                    nlohmann::json response = nlohmann::json::parse(receive_response());
                    
                    std::cout << "Server response status: ";
                    if(response["status"] == "success") {
                        std::cout << "Success: " << response["message"] << std::endl;
                        user.set_id(response["user_id"].get<int>());
                        user.set_nickname(nickname);
                        user.set_password(password);
                    } else if (response["status"] == "error") {
                        std::cout << "Error: " << response["message"] << std::endl;
                    }
                    
                    DEBUG_MSG("Server response: " + response.dump());
                    break;                
                }
                case 1: {
                    if (user.get_id() == 0) {
                        std::cout << "Error: You need to register first." << std::endl;
                        break;
                    }
                    
                    std::string password;
                    std::cout << user.get_nickname() << ", please, enter password: ";
                    std::getline(std::cin, password);
                    
                    request["password"] = password;
                    request["type"] = "authorize";
                    request["nickname"] = user.get_nickname();
                    request["user_id"] = user.get_id();

                    boost::asio::write(socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
                    nlohmann::json response = nlohmann::json::parse(receive_response());
                    
                    std::cout << "Server response status: ";
                    if(response["status"] == "success") {
                        std::cout << "Success: " << response["message"] << std::endl;
                    } else if (response["status"] == "error") {
                        std::cout << "Error: " << response["message"] << std::endl;
                    }
                    
                    DEBUG_MSG("Server response: " + response.dump());
                    break;                
                }
                case 2: {
                    std::cout << "TODO: " << std::endl;
                    std::cout << "Getting list of all chats... " << std::endl;
                    std::cout << std::endl;
                    break;
                }
                case 3: {
                    std::string receiver, message;
                    // remove when chat logic will be implemented?
                    std::cout << "Provide receiver nickname: ";
                    std::getline(std::cin, receiver);
                    
                    std::cout << "Provide the message for " << receiver << ": ";
                    std::getline(std::cin, message);
                    
                    request["type"] = "send_message";
                    request["sender"] = user.get_nickname();
                    request["receiver"] = receiver;
                    request["message"] = message;
                    
                    
                    // this should be in a separate func or at least out side of the case {}
                    boost::asio::write(socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
                    response = nlohmann::json::parse(receive_response());
                    
                    std::cout << "Server response status is ";
                    if(response["status"] == "success") {
                       std::cout << "success: " << response["message"] << std::endl;
                    } else if (response["status"] == "error") {
                        std::cout << "error: " << response["message"] << std::endl;
                    }

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
            "Error while receiving response: " + error.message());
    }

    std::istream response_stream(&response_buf);
    std::string response(
        (std::istreambuf_iterator<char>(response_stream)),
        std::istreambuf_iterator<char>()
    );

    return response;
}
