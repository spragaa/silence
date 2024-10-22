#include "client.hpp"

Client::Client(const std::string& server_address,
               unsigned short server_port, const std::string& nick)
	: _io_service()
	, _socket(_io_service)
	, _work(new boost::asio::io_service::work(_io_service))
	, _server_address(server_address)
	, _server_port(server_port)
	, _user(nick)
	, _is_authorized(false)
{
	DEBUG_MSG("[Client::Client] Trying to read user data from json...");
	std::string user_data_filename = get_user_data_filename();
	_user = User::load_user_data_from_json(user_data_filename);
	if (_user.get_id() == 0) {
		_user = User();
		_user.set_nickname(nick);
		DEBUG_MSG("[Client::Client] Temporary user created with nickname: '" + _user.get_nickname() + "'");
	}
}

Client::~Client() {
	_work.reset();
	_io_service.stop();
	for (auto& thread : _io_threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
}

User Client::get_user() {
	return _user;
}


void inline Client::show_actions() {
	std::cout << _user.get_nickname() << ", please select the number of the action you would like to perform now from the list below:" << std::endl;
	std::cout << "0. Send message" << std::endl;
	std::cout << "1. Get list of chats" << std::endl;
	std::cout << "2. Get offline messages" << std::endl;
	std::cout << "5. Exit" << std::endl;

	std::cout << "Input: ";
}

bool Client::is_registered() const noexcept {
	return _user.get_id() != 0;
}

void Client::register_user() {
	std::string password;
	std::string nickname = _user.get_nickname();
	std::cout << nickname << ", create password: ";
	std::getline(std::cin, password);

	nlohmann::json request;
	request["type"] = "register";
	request["nickname"] = nickname;
	request["password"] = password;

	boost::asio::write(_socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
	DEBUG_MSG("[Client::register_user] Sending request: " + request.dump());
	nlohmann::json response = nlohmann::json::parse(receive_response());
	DEBUG_MSG("[Client::register_user] Received response: " + response.dump());

	if(response["status"] == "success") {
		if (response.contains("user_id") && !response["user_id"].is_null()) {
			_user.set_id(response["user_id"].get<int>());
			DEBUG_MSG("[Client::register_user] User id set: " + std::to_string(_user.get_id()));
		} else {
			WARN_MSG("[Client::register_user] User ID not provided in the response");
		}
		if (response.contains("registered_timestamp") && !response["registered_timestamp"].is_null()) {
			std::chrono::nanoseconds ns(response["registered_timestamp"].get<int64_t>());
			Timestamp registered_timestamp = std::chrono::system_clock::time_point(ns);
			_user.set_registered_timestamp(registered_timestamp);
		} else {
			WARN_MSG("[Client::register_user] Registered timestamp not provided in the response");
		}

		_user.set_nickname(nickname);
		_user.set_password(password);
		_user.save_user_data_to_json(get_user_data_filename());
		INFO_MSG("[Client::register_user] Registration successful. You can now authorize.")
	} else if (response["status"] == "error") {
		ERROR_MSG("[Client::register_user] Failed to register!");
	}
}

void Client::authorize_user() {
	std::string password;
	std::cout << _user.get_nickname() << ", please enter password: ";
	std::getline(std::cin, password);

	nlohmann::json request;
	request["type"] = "authorize";
	request["password"] = password;
	request["nickname"] = _user.get_nickname();
	request["user_id"] = _user.get_id();
	DEBUG_MSG("[Client::authorize_user()] Sending request:" + request.dump());

	try {
		boost::asio::write(_socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
		INFO_MSG("[Client::authorize_user()] Authorization request sent successfully");

		nlohmann::json response = nlohmann::json::parse(receive_response());
		DEBUG_MSG("[Client::authorize_user()] Received response: " + response.dump());

		if(response["status"] == "success") {
			_is_authorized = true;
			// if (response.contains("user_data") && !response["user_data"].is_null()) {
			// nlohmann::json user_data = response["user_data"];
			// this is not yet finished on server side
			// if (user_data.contains("last_online_timestamp") && !user_data["last_online_timestamp"].is_null()) {
			//     std::chrono::nanoseconds ns(user_data["last_online_timestamp"].get<int64_t>());
			//     Timestamp last_online = std::chrono::system_clock::time_point(ns);
			//     _user.set_last_online_timestamp(last_online);
			// }
			// if (user_data.contains("is_online") && !user_data["is_online"].is_null()) {
			//     _user.set_online(user_data["is_online"].get<bool>());
			// }
			// }

			_user.save_user_data_to_json(get_user_data_filename());
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

void Client::async_read() {
	DEBUG_MSG("[Client::async_read] Async read has been started");

	_io_service.post([this]() {
		boost::asio::async_read_until(_socket, _read_buffer, "\r\n\r\n",
		                              boost::bind(&Client::handle_async_read, this,
		                                          boost::asio::placeholders::error,
		                                          boost::asio::placeholders::bytes_transferred));
	});
}

void Client::handle_async_read(const boost::system::error_code& error, size_t bytes_transferred) {
	if (!error) {
		std::string message(boost::asio::buffers_begin(_read_buffer.data()),
		                    boost::asio::buffers_begin(_read_buffer.data()) + bytes_transferred);
		_read_buffer.consume(bytes_transferred);
		DEBUG_MSG("[Client::handle_async_read] Read: " + message);

		process_server_message(message);

		async_read();
	} else if (error == boost::asio::error::eof) {
		ERROR_MSG("[Client::handle_async_read] Server closed connection");
		// disconnect
	} else {
		ERROR_MSG("[Client::handle_async_read] " + error.message());
		// reconnect or handle error correcrlty
	}
}

bool Client::is_connected() {
	return _socket.is_open();
}

void Client::async_write(const std::string& message) {
	if (!is_connected()) {
		ERROR_MSG("[Client::async_write] Not connected to server");
		return;
	}

	_io_service.post([this, message]() {
		bool write_in_progress = !_write_queue.empty();
		_write_queue.push(message + "\r\n\r\n");

		if (!write_in_progress) {
			do_write();
		}
	});
}

void Client::do_write() {
	if (!_write_queue.empty()) {
		boost::asio::async_write(_socket,
		                         boost::asio::buffer(_write_queue.front()),
		                         boost::bind(&Client::handle_async_write, this,
		                                     boost::asio::placeholders::error));
	}
}

void Client::handle_async_write(const boost::system::error_code& error) {
	if (!error) {
		_write_queue.pop();
		do_write();
	} else {
		ERROR_MSG("[Client::handle_async_write] " + error.message());
		// recconect
		// inform user
	}
}

void Client::process_server_message(const std::string& message) {
	try {
		nlohmann::json json_message = nlohmann::json::parse(message);
		DEBUG_MSG("[Client::process_server_message] Parsed response: " + json_message.dump());
	} catch (const nlohmann::json::parse_error& e) {
		ERROR_MSG("[Client::process_server_message] Failed to parse message: " + std::string(e.what()));
	}
}

// should be async!
// void Client::send_file_chunks(const std::string& filepath) {
//     std::ifstream file(filepath, std::ios::binary);
//     if (!file) {
//         ERROR_MSG("[Client::send_file_chunks] Unable to open file: " + filepath);
//         return;
//     }

//     const size_t chunk_size = 512;
//     std::vector<char> buffer(chunk_size);
//     size_t chunk_number = 0;

//     while (!file.eof()) {
//         file.read(buffer.data(), chunk_size);
//         std::streamsize bytes_read = file.gcount();

//         if (bytes_read == 0) {
//             break;
//         }

//         nlohmann::json file_chunk;
//         file_chunk["type"] = "file_chunk";
//         file_chunk["filename"] = fs::path(filepath).filename().string();
//         file_chunk["chunk_data"] = std::string(buffer.data(), bytes_read);
//         file_chunk["is_last"] = file.eof();

//         async_write(file_chunk.dump());
//     }
// }

void Client::handle_user_interaction() {
	DEBUG_MSG("[Client::handle_user_interaction()] Starting interaction with user!");

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

		// my idea is to use request statuses???
		// after client registered or/and authorized
		// client adds another thread (potentially more),
		// main thread is sending requests, and new thread is receiving responses
		//
		switch (action_type) {
				case 0: {
					std::cout << "Enter recipient's nickname: ";
					std::string recipient;
					std::getline(std::cin, recipient);
					std::cout << "Enter your message: ";
					std::string message_text;
					std::getline(std::cin, message_text);
					
					// how send multiple files?
					std::string file_name;
					std::cout << "If there is a file to send, please enter the file name (it should be located in " 
					          << _user_files_dir << " or type no, otherwise: ";
					std::getline(std::cin, file_name);

					nlohmann::json message;
					message["type"] = "send_message";
					message["sender_id"] = _user.get_id();
					message["receiver_nickname"] = recipient;
					message["message_text"] = message_text;
					
					// not sure if filepath is the best way of doing it
					message["filepath"] = "none";
					fs::path filepath = fs::path(_user_files_dir) / file_name;
					if (!fs::exists(filepath)) {
					    WARN_MSG("[Client::handle_user_interaction] File doesn't exist: " + filepath.string());
					} else {
					    message["filepath"] = filepath.string();
					}					

					async_write(message.dump());
					// async?
					// send_file(filepath);
					
					DEBUG_MSG("CASE 0");
				}
				break;
				case 1:
					DEBUG_MSG("CASE 1");
					break;
				case 2:
					DEBUG_MSG("CASE 2");
					break;
				case 3:
					DEBUG_MSG("CASE 3");
					break;
				case 4:
					DEBUG_MSG("CASE 4");
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
		boost::asio::ip::tcp::resolver resolver(_io_service);
		boost::asio::ip::tcp::resolver::query query(_server_address, std::to_string(_server_port));
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		boost::asio::connect(_socket, endpoint_iterator);

		unsigned int thread_count = std::thread::hardware_concurrency();
		for (unsigned int i = 0; i < thread_count; ++i) {
			_io_threads.emplace_back([this]() {
				_io_service.run();
			});
		}

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

		while (!_is_authorized) {
			INFO_MSG("[Client::run()] Please authorize to continue.");

			authorize_user();
			if (!_is_authorized) {
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
		async_read();
		handle_user_interaction();

		_work.reset();
		for (auto& thread : _io_threads) {
			thread.join();
		}
		_socket.close();
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
	DEBUG_MSG("send_message" + get_socket_info(_socket));
	boost::asio::write(_socket, boost::asio::buffer(message + "\r\n\r\n"));
}

std::string Client::receive_response() {
	boost::asio::streambuf response_buf;
	boost::system::error_code error;

	boost::asio::read_until(_socket, response_buf, "\r\n\r\n", error);

	if (error && (error != boost::asio::error::eof)) {
		ERROR_MSG("[Client::receive_response()] While receiving response: " + std::string(error.message()));
	}

	std::istream response_stream(&response_buf);
	std::string response(
		(std::istreambuf_iterator<char>(response_stream)),
		std::istreambuf_iterator<char>()
		);

	return response;
}

std::string Client::get_user_data_filename() const noexcept {
	return std::string(SOURCE_DIR) + "/client/user_data/" + "user_" + _user.get_nickname() + "_" + _server_address + "_" + std::to_string(_server_port) + ".json";
}