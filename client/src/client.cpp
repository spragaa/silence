#include "client.hpp"

Client::Client(const std::string& server_address,
               unsigned short server_port, const std::string& nick)
	: io_service()
	, socket(io_service)
	, server_address(server_address)
	, server_port(server_port)
	, user(nick)
{
}

User Client::get_user() {
	return user;
}

void inline Client::show_actions() {
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

		std::string user_data_filename = get_user_data_filename();

		try {
			user = User::load_user_data_from_json(user_data_filename);
			std::cout << "User data loaded successfully." << std::endl;
		} catch (const std::exception& e) {
			std::cout << "No existing user data found. Please register." << std::endl;
		}

		// TODO: remembder nickname on the client side
		while (true) {
			bool is_registered = (user.get_id() != 0);
			int action_type = -1;

			if (!is_registered) {
				std::cout << "You need to register first." << std::endl;
				action_type = 0;
			} else {
				while (true) {
					show_actions();
					std::cout << "Choose an action (0-5): ";
					std::string input;
					std::getline(std::cin, input);

					//somehow here empty line is printed and nothing happens
					try{
						action_type = std::stoi(input);
					} catch (std::invalid_argument) {
						action_type = -1;
					}

					if (action_type >= 0 && action_type < 6) {
						break;
					}

					std::cout << "Invalid input. Please enter a number between 0 and 5." << std::endl;
					show_actions();
				}

				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			}

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
							std::cout << "Success: " << response["response"] << std::endl;
							if (response.contains("user_id") && !response["user_id"].is_null()) {
								user.set_id(response["user_id"].get<int>());
							} else {
								std::cout << "Warning: User ID not provided in the response" << std::endl;
							}
							if (response.contains("registered_timestamp") && !response["registered_timestamp"].is_null()) {
								std::chrono::nanoseconds ns(response["registered_timestamp"].get<int64_t>());
								Timestamp registered_timestamp = std::chrono::system_clock::time_point(ns);
								user.set_registered_timestamp(registered_timestamp);
							} else {
								std::cout << "Warning: User ID not provided in the response" << std::endl;
							}

							user.set_nickname(nickname);
							user.set_password(password);
							user.save_user_data_to_json(get_user_data_filename());
							std::cout << "Registration successful. You can now use other actions" << std::endl;
						} else if (response["status"] == "error") {
							std::cout << "Error: " << response["response"] << std::endl;
							std::cout << "Please try registering again." << std::endl;
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
						std::cout << user.get_nickname() << ", please enter password: ";
						std::getline(std::cin, password);

						nlohmann::json request;
						request["type"] = "authorize";
						request["password"] = password;
						request["nickname"] = user.get_nickname();
						request["user_id"] = user.get_id();

						DEBUG_MSG("Sending authorization request: " + request.dump());

						try {
							boost::asio::write(socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
							INFO_MSG("Authorization request sent successfully");

							nlohmann::json response = nlohmann::json::parse(receive_response());

							if(response["status"] == "success") {
								std::cout << "Success: " << response["message"] << std::endl;

								if (response.contains("user_data") && !response["user_data"].is_null()) {
									nlohmann::json user_data = response["user_data"];
									if (user_data.contains("last_online_timestamp") && !user_data["last_online_timestamp"].is_null()) {
										std::chrono::nanoseconds ns(user_data["last_online_timestamp"].get<int64_t>());
										Timestamp last_online = std::chrono::system_clock::time_point(ns);
										user.set_last_online_timestamp(last_online);
									}
									if (user_data.contains("is_online") && !user_data["is_online"].is_null()) {
										user.set_online(user_data["is_online"].get<bool>());
									}
								}

								user.save_user_data_to_json(get_user_data_filename());

								std::cout << "User data updated and saved" << std::endl;
							} else if (response["status"] == "error") {
								std::cout << "Error: " << response["message"] << std::endl;
							} else {
								std::cout << "Unexpected response from server" << std::endl;
							}
						} catch (const std::exception& e) {
							std::cerr << "Exception during authorization: " << e.what() << std::endl;
						}

						INFO_MSG("Authorization process completed");
						break;
					}
					case 2: {
						std::cout << "TODO: " << std::endl;
						std::cout << "Getting list of all chats... " << std::endl;
						std::cout << std::endl;
						break;
					}
					// A sends a message to B
					// on server we store a message from A to B with all needed metadata
					// client has its own copy of the message stored,
					// lets say that client can store only last N messages
					// to open chat, it make request to the server to get last M messages
					// to do so, server find last M messages where receiver or/and sender is A and B in the messages table
					//
					// ? on the client side I would like to have chats list ?
					case 3: {
						// move somewhere
						if (user.get_id() == 0) {
							std::cout << "Error: You need to register and authorize first." << std::endl;
							break;
						}

						std::string receiver_nickname, message_text;
						std::cout << "Provide receiver nickname: ";
						std::getline(std::cin, receiver_nickname);

						std::cout << "Provide the message for " << receiver_nickname << ": ";
						std::getline(std::cin, message_text);

						request["type"] = "send_message";
						request["sender_id"] = user.get_id();
						request["sender_nickname"] = user.get_nickname();
						request["receiver_nickname"] = receiver_nickname;
						request["message_text"] = message_text;
						// handle mediahere

						boost::asio::write(socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
						nlohmann::json response = nlohmann::json::parse(receive_response());

						if(response["status"] == "success") {
							std::cout << "Message sent successfully." << std::endl;
							Message received_message = Message::from_json(response["message"]);
							messages.push_back(received_message);
							DEBUG_MSG("Message added: " + received_message.to_json().dump());
						} else if (response["status"] == "error") {
							std::cout << "Error: " << response["message"] << std::endl;
						}

						INFO_MSG("Server response: " + response.dump());

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
						if (!is_registered) {
							std::cout << "You must register before performing other actions." << std::endl;
							break;
						}
					}
			}
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

std::string Client::get_user_data_filename() const noexcept {
	return std::string(SOURCE_DIR) + "/user_data/" + "user_ " + user.get_nickname() + "_" + server_address + "_" + std::to_string(server_port) + ".json";
}