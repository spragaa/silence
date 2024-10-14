#include "client.hpp"

Client::Client(const std::string& server_address,
               unsigned short server_port, const std::string& nick)
	: io_service()
	, socket(io_service)
	, server_address(server_address)
	, server_port(server_port)
	, user(nick)
	, is_authorized(false)
{
	DEBUG_MSG("[Client::Client] Trying to read user data from json...");
	std::string user_data_filename = get_user_data_filename();
	user = User::load_user_data_from_json(user_data_filename);
	if (user.get_id() == 0) {
		user = User();
		user.set_nickname(nick);
		DEBUG_MSG("[Client::Client] Temporary user created with nickname: '" + user.get_nickname() + "'");
	}
}

User Client::get_user() {
	return user;
}

void inline Client::show_actions() {
	std::cout << user.get_nickname() << ", please select the number of the action you would like to perform now from the list below:" << std::endl;
	std::cout << "2. Get list of chats" << std::endl;
	std::cout << "3. Send message" << std::endl;
	std::cout << "4. Get offline messages" << std::endl;
	std::cout << "5. Exit" << std::endl;

	std::cout << "Input: ";
}

bool Client::is_registered() const noexcept {
	return user.get_id() != 0;
}

void Client::register_user() {
	std::string password;
	std::string nickname = user.get_nickname();
	std::cout << nickname << ", create password: ";
	std::getline(std::cin, password);

	nlohmann::json request;
	request["type"] = "register";
	request["nickname"] = nickname;
	request["password"] = password;

	boost::asio::write(socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
	DEBUG_MSG("[Client::register_user] Sending request: " + request.dump());
	nlohmann::json response = nlohmann::json::parse(receive_response());
	DEBUG_MSG("[Client::register_user] Received response: " + response.dump());

	if(response["status"] == "success") {
		if (response.contains("user_id") && !response["user_id"].is_null()) {
			user.set_id(response["user_id"].get<int>());
			DEBUG_MSG("[Client::register_user] User id set: " + std::to_string(user.get_id()));
		} else {
			WARN_MSG("[Client::register_user] User ID not provided in the response");
		}
		if (response.contains("registered_timestamp") && !response["registered_timestamp"].is_null()) {
			std::chrono::nanoseconds ns(response["registered_timestamp"].get<int64_t>());
			Timestamp registered_timestamp = std::chrono::system_clock::time_point(ns);
			user.set_registered_timestamp(registered_timestamp);
		} else {
			WARN_MSG("[Client::register_user] Registered timestamp not provided in the response");
		}

		user.set_nickname(nickname);
		user.set_password(password);
		user.save_user_data_to_json(get_user_data_filename());
		INFO_MSG("[Client::register_user] Registration successful. You can now authorize.")
	} else if (response["status"] == "error") {
		ERROR_MSG("[Client::register_user] Failed to register!");
	}
}

void Client::authorize_user() {
	std::string password;
	std::cout << user.get_nickname() << ", please enter password: ";
	std::getline(std::cin, password);

	nlohmann::json request;
	request["type"] = "authorize";
	request["password"] = password;
	request["nickname"] = user.get_nickname();
	request["user_id"] = user.get_id();
	DEBUG_MSG("[Client::authorize_user()] Sending request:" + request.dump());

	try {
		boost::asio::write(socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
		INFO_MSG("[Client::authorize_user()] Authorization request sent successfully");

		nlohmann::json response = nlohmann::json::parse(receive_response());
		DEBUG_MSG("[Client::authorize_user()] Received response: " + response.dump());

		if(response["status"] == "success") {
			is_authorized = true;
			// if (response.contains("user_data") && !response["user_data"].is_null()) {
			// nlohmann::json user_data = response["user_data"];
			// this is not yet finished on server side
			// if (user_data.contains("last_online_timestamp") && !user_data["last_online_timestamp"].is_null()) {
			//     std::chrono::nanoseconds ns(user_data["last_online_timestamp"].get<int64_t>());
			//     Timestamp last_online = std::chrono::system_clock::time_point(ns);
			//     user.set_last_online_timestamp(last_online);
			// }
			// if (user_data.contains("is_online") && !user_data["is_online"].is_null()) {
			//     user.set_online(user_data["is_online"].get<bool>());
			// }
			// }

			user.save_user_data_to_json(get_user_data_filename());
			INFO_MSG("[Client::authorize_user()] User data updated and saved");
		} else if (response["status"] == "error") {
			ERROR_MSG("[Client::authorize_user()] Failed to authorize user!");
		} else {
			ERROR_MSG("[Client::authorize_user()] Unexpected response from server");
		}
	} catch (const std::exception& e) {
		ERROR_MSG("[Client::authorize_user()] Exception during authorization" + std::string(e.what()));
	}

	INFO_MSG("[Client::authorize_user()] Authorization process completed");
}

void Client::handle_user_actions() {
	while (true) {
		show_actions();
		int action_type = -1;

		while (true) {
			std::cout << "Choose an action (0-5): ";
			std::string input;
			std::getline(std::cin, input);

			try {
				action_type = std::stoi(input);
			} catch (std::invalid_argument) {
				action_type = -1;
			}

			if (action_type >= 0 && action_type < 6) {
				break;
			}

			std::cout << "Invalid input. Please enter a number between 0 and 5." << std::endl;
		}

		switch (action_type) {
				case 0:
					std::cout << "You are already registered and authorized." << std::endl;
					break;
				case 1:
					std::cout << "You are already authorized" << std::endl;
					break;
				case 2:
					std::cout << "TODO: Implement get list of chats" << std::endl;
					break;
				case 3:
					std::cout << "TODO: Implement send message" << std::endl;
					break;
				case 4:
					std::cout << "TODO: Implement get offline messages" << std::endl;
					break;
				case 5:
					std::cout << "Exiting..." << std::endl;
					return;
				default:
					std::cout << "Invalid action. Please try again." << std::endl;
		}
	}
}

void Client::run() {
	try {
		boost::asio::ip::tcp::resolver resolver(io_service);
		boost::asio::ip::tcp::resolver::query query(server_address, std::to_string(server_port));
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		boost::asio::connect(socket, endpoint_iterator);

		while (!is_registered()) {
			WARN_MSG("[Client::run()] No existing user data found. Please register.");
			register_user();
			if (!is_registered()) {
				WARN_MSG("[Client::run()] Registration failed. Would you like to try again? (y/n):");
				std::string response;
				std::getline(std::cin, response);
				if (response != "y" && response != "Y") {
					INFO_MSG("[Client::run()] Exiting...");
					return;
				}
			}
		}

		INFO_MSG("[Client::run()] User registered successfully");

		while (!is_authorized) {
			INFO_MSG("[Client::run()] Please authorize to continue.");

			authorize_user();
			if (!is_authorized) {
				WARN_MSG("[Client::run()] Authorization failed. Would you like to try again? (y/n): ");
				std::string response;
				std::getline(std::cin, response);
				if (response != "y" && response != "Y") {
					INFO_MSG("[Client::run()] Exiting...");
					return;
				}
			}
		}

		INFO_MSG("[Client::run()] User authorized successfully");
		INFO_MSG("[Client::run()] Now you can start messaging!");
		handle_user_actions();

		socket.close();
	} catch (const std::exception& e) {
		ERROR_MSG("[Client::run()] " + std::string(e.what()));
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
	return std::string(SOURCE_DIR) + "/user_data/" + "user_" + user.get_nickname() + "_" + server_address + "_" + std::to_string(server_port) + ".json";
}