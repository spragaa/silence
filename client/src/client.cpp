#include "client.hpp"

namespace client {

constexpr uint8_t filename_len = 16;
inline const std::string alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

Client::Client(const std::string& server_address,
               unsigned short server_port, const std::string& nick)
	: _io_service()
	, _socket(_io_service)
	, _work(new boost::asio::io_service::work(_io_service))
	, _server_address(server_address)
	, _server_port(server_port)
	, _user(nick)
	, _is_authorized(false)
	, _user_files_dir(std::string(SOURCE_DIR) + "/client/user_files/" + _user.get_nickname())
{
	std::filesystem::create_directories(_user_files_dir);

	DEBUG_MSG("[Client::Client] Trying to read user data from json...");
	std::string user_data_filename = get_user_data_filename();
	_user = common::User::load_user_data_from_json(user_data_filename);
	if (_user.get_id() == 0) {
		_user = common::User();
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


void inline Client::show_actions() {
	std::cout << _user.get_nickname() << ", please select the number of the action you would like to perform now from the list below:" << std::endl;
	std::cout << "0. Send message" << std::endl;
	std::cout << "1. Get user public keys" << std::endl;
	std::cout << "2. Share file with chat application" << std::endl;
	std::cout << "5. Exit" << std::endl;

	std::cout << "Input: ";
}

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

					// how to send multiple files?
					std::string file_name;
					std::cout << "If there is a file to send, please enter the file name (it should be located in "
					          << _user_files_dir << " or type no, otherwise: ";
					std::getline(std::cin, file_name);

					// aes key exchange should only be called on first message in a chat
					// but currently we don't have the chat logic, so we will do it on each message
					// if (!send_aes_key(recipient)) {
					// 	FATAL_MSG("[Client::handle_user_interaction] Failed to exchange aes keys for safe message exchange with " + recipient);
					// 	// add retry logic?
					// 	return;
					// }

					// sending the actual message
					nlohmann::json message;
					message["type"] = "send_message";
					message["sender_id"] = _user.get_id();
					message["receiver_nickname"] = recipient;
					message["message_text"] = message_text;

					// not sure if filepath is the best way of doing it
					message["file_name"] = "none";
					std::filesystem::path filepath = std::filesystem::path(_user_files_dir) / file_name;
					if (!std::filesystem::exists(filepath)) {
						WARN_MSG("File doesn't exist: " + filepath.string());
						WARN_MSG("Sending message without file");
					} else {
						message["file_name"] = file_name;
					}

					async_write(message.dump());
					// should be async
					send_file_chunks(filepath);
					break;
				}
				case 1: { // will remove in future, added for testing purposes
					DEBUG_MSG("Getting users crypto keys...");
					std::cout << "Enter user nickname: ";
					std::string u1;
					std::getline(std::cin, u1);

					get_receiver_public_keys(u1);

					break;
				}
				case 2: {
					std::cout << "Enter file path: ";
					std::string input;
					std::getline(std::cin, input);

					std::filesystem::path filepath = std::filesystem::path(input);

					if (!std::filesystem::exists(filepath)) {
						WARN_MSG("File doesn't exist: " + filepath.string());
						break;
					}

					std::string random_filename = generate_random_string(filename_len) +
					                              filepath.extension().string();
					std::filesystem::path destination = std::filesystem::path(_user_files_dir) / random_filename;

					try {
						std::filesystem::copy_file(filepath, destination);
						INFO_MSG("File " + filepath.string() + " was moved successfully and renamed to: " + random_filename);
					} catch (const std::filesystem::filesystem_error& e) {
						ERROR_MSG("Error while copying file: " + std::string(e.what()));
					}
					break;
				}
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

std::string Client::read_user_text() const noexcept{
	std::string message;
	std::getline(std::cin, message);

	return message;
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
	request["el_gamal_public_key"] = crypto::cpp_int_to_hex(_hybrid_crypto_system.get_el_gamal_public_key());
	request["dsa_public_key"] = crypto::cpp_int_to_hex(_hybrid_crypto_system.get_dsa_public_key());

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
			common::Timestamp registered_timestamp = std::chrono::system_clock::time_point(ns);
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
	request["user_id"] = _user.get_id();
	request["nickname"] = _user.get_nickname();
	request["password"] = password;

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
			//     common::Timestamp last_online = std::chrono::system_clock::time_point(ns);
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

// void?
void Client::send_aes_key(const std::string& receiver_nickname, const std::string& receiver_public_key) {
	crypto::EncryptedMessage encrypted_aes_key = _hybrid_crypto_system.encrypt_aes_key(receiver_public_key);
	std::string encrypted_aes_key_c1 = crypto::cpp_int_to_hex(encrypted_aes_key._c1);
	std::string encrypted_aes_key_c2 = crypto::cpp_int_to_hex(encrypted_aes_key._c2);

	crypto::SHA256 sha256;
	sha256.update(encrypted_aes_key_c1 + encrypted_aes_key_c2);
	crypto::DSASignature dsa_signature = _hybrid_crypto_system.sign(
		crypto::hex_to_cpp_int(sha256.digest())
		);

	nlohmann::json request;
	request["type"] = "send_aes_key";
	request["receiver_nickname"] = receiver_nickname;
	request["encrypted_aes_key_c1"] = encrypted_aes_key_c1;
	request["encrypted_aes_key_c2"] = encrypted_aes_key_c2;
	request["dsa_r"] = crypto::cpp_int_to_hex(dsa_signature._r);
	request["dsa_signature"] = crypto::cpp_int_to_hex(dsa_signature._signature);

	DEBUG_MSG("[Client::send_aes_key] Sending request: " + request.dump());

	try {
		boost::asio::write(_socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
	} catch (const std::exception& e) {
		ERROR_MSG("[Client::send_aes_key] Exception caught: " + std::string(e.what()));
	}
}

void Client::get_receiver_public_keys(const std::string& receiver_nickname) {
	nlohmann::json request;
	request["type"] = "get_user_keys";
	request["nickname"] = receiver_nickname;

	DEBUG_MSG("[Client::get_receiver_public_keys()] Sending request: " + request.dump());

	try {
		boost::asio::write(_socket, boost::asio::buffer(request.dump() + "\r\n\r\n"));
	} catch (const std::exception& e) {
		ERROR_MSG("[Client::get_receiver_public_keys()] Exception caught: " + std::string(e.what()));
	}
}

void Client::send_message(const std::string& message) {
	DEBUG_MSG("send_message" + common::get_socket_info(_socket));
	boost::asio::write(_socket, boost::asio::buffer(message + "\r\n\r\n"));
}

void Client::send_file_chunks(const std::string& filepath) {
	std::ifstream file(filepath, std::ios::binary);
	if (!file) {
		ERROR_MSG("[Client::send_file_chunks] Unable to open file: " + filepath);
		return;
	}

	const size_t chunk_size = 512;
	std::vector<char> buffer(chunk_size);
	// get file size -> calculate the amount of chunk mark the chunk as the last when chunk_number is 0???
	size_t chunk_number = 1;
	size_t total_bytes_sent = 0;

	INFO_MSG("[Client::send_file_chunks] Starting to send file: " + filepath);

	while (file) {
		file.read(buffer.data(), chunk_size);
		std::streamsize bytes_read = file.gcount();

		if (bytes_read <= 0) {
			break;
		}

		total_bytes_sent += bytes_read;
		bool is_last = file.eof() || bytes_read < chunk_size;

		nlohmann::json file_chunk_request;
		file_chunk_request["type"] = "file_chunk";
		file_chunk_request["filename"] = std::filesystem::path(filepath).filename().string();
		file_chunk_request["chunk_data"] = std::string(buffer.data(), bytes_read);
		file_chunk_request["chunk_number"] = chunk_number;
		file_chunk_request["is_last"] = is_last;

		DEBUG_MSG("[Client::send_file_chunks] Sending chunk " + std::to_string(chunk_number) +
		          " (bytes: " + std::to_string(bytes_read) +
		          ", is_last: " + (is_last ? "true" : "false") + ")");

		async_write(file_chunk_request.dump());

		{
			std::unique_lock<std::mutex> lock(_mutex);
			if (!_chunk_cv.wait_for(lock,
			                        std::chrono::seconds(5),
			                        [this] {
					return _chunk_acknowledged;
				})) {
				ERROR_MSG("[Client::send_file_chunks] Timeout waiting for chunk acknowledgment");
				return;
			}
			_chunk_acknowledged = false;
		}

		chunk_number++;
	}

	INFO_MSG("[Client::send_file_chunks] File transfer completed: " +
	         filepath + " (" +
	         std::to_string(total_bytes_sent) + " bytes in " +
	         std::to_string(chunk_number - 1) + " chunks)");
}

void Client::send_next_chunk(std::shared_ptr<FileTransferState> state) {
	if (!state->file) {
		return;
	}

	state->file.read(state->buffer.data(), state->chunk_size);
	std::streamsize bytes_read = state->file.gcount();

	if (bytes_read <= 0) {
		state->file.close();
		DEBUG_MSG("[Client::send_next_chunk] File transfer completed");
		return;
	}

	nlohmann::json chunk_request;
	chunk_request["type"] = "file_chunk";
	chunk_request["filename"] = state->filename;
	chunk_request["chunk_data"] = std::string(state->buffer.data(), bytes_read);
	chunk_request["chunk_number"] = state->chunk_number + 1;
	chunk_request["is_last"] = state->file.eof();

	DEBUG_MSG("[Client::send_next_chunk] Sending chunk " + std::to_string(state->chunk_number + 1));

	async_write(chunk_request.dump());
	wait_for_chunk_ack(state);
}

void Client::wait_for_chunk_ack(std::shared_ptr<FileTransferState> state) {
	auto timer = std::make_shared<boost::asio::steady_timer>(_io_service,
	                                                         std::chrono::milliseconds(50));

	// ec is needed here
	timer->async_wait([this, timer, state](const boost::system::error_code& /*ec*/) {
			std::lock_guard<std::mutex> lock(_mutex);
			if (_chunk_acknowledged) {
				_chunk_acknowledged = false;
				state->chunk_number++;
				send_next_chunk(state);
			} else {
				wait_for_chunk_ack(state);
			}
		});
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

void Client::async_write(const std::string& message) {
	DEBUG_MSG("[Client::async_write] Tryign to write request: " + message);

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

void Client::process_server_message(const std::string& message) {
	try {
		nlohmann::json json_message = nlohmann::json::parse(message);
		DEBUG_MSG("[Client::process_server_message] Parsed response: " + json_message.dump());

		if (json_message["type"] == "chunk_acknowledgment") {
			handle_chunk_acknowledgment(json_message);
		}
		else if (json_message["type"] == "incoming_file") {
			handle_incoming_file(json_message);
		}
		else if (json_message["type"] == "file_chunk") {
			handle_incoming_file_chunk(json_message);
		} else if (json_message["type"] == "receive_user_keys") {
			handle_receive_user_keys(json_message);
		}
	} catch (const nlohmann::json::parse_error& e) {
		ERROR_MSG("[Client::process_server_message] Failed to parse message: " + std::string(e.what()));
	}
}

void Client::handle_chunk_acknowledgment(const nlohmann::json& response) {
	if (response["status"] == "success") {
		std::lock_guard<std::mutex> lock(_mutex);
		_chunk_acknowledged = true;
		_chunk_cv.notify_one();
	} else {
		ERROR_MSG("[Client::handle_chunk_acknowledgment] Chunk upload failed: "
		          + response["error"].get<std::string>());
	}
}

void Client::handle_incoming_file(const nlohmann::json& notification) {
	std::string filename = notification["filename"];
	int sender_id = notification["sender_id"];

	INFO_MSG("[Client::handle_incoming_file] Receiving file '" + filename
	         + "' from " + std::to_string(sender_id));

	_incoming_files[filename] = std::ofstream(
		_user_files_dir + "/" + filename,
		std::ios::binary
		);
}

void Client::handle_incoming_file_chunk(const nlohmann::json& chunk_message) {
	std::string filename = chunk_message["filename"];
	std::string chunk_data = chunk_message["chunk_data"];
	size_t chunk_number = chunk_message["chunk_number"];
	bool is_last = chunk_message["is_last"];

	auto file_it = _incoming_files.find(filename);
	if (file_it == _incoming_files.end()) {
		ERROR_MSG("[Client::handle_incoming_file_chunk] No open file for " + filename);
		return;
	}

	file_it->second.write(chunk_data.c_str(), chunk_data.length());

	nlohmann::json ack;
	ack["type"] = "chunk_received";
	ack["filename"] = filename;
	ack["chunk_number"] = chunk_number;
	async_write(ack.dump());

	if (is_last) {
		INFO_MSG("[Client::handle_incoming_file_chunk] File " + filename + " received completely");
		file_it->second.close();
		_incoming_files.erase(file_it);
	}
}

void Client::handle_receive_user_keys(const nlohmann::json& response) {
	if (response["type"] == "success") {
		int user_id = response["sender_id"];
		crypto::UserCryptoKeys user_crypto_keys(
			crypto::hex_to_cpp_int(response["dsa_public_key"]),
			crypto::hex_to_cpp_int(response["el_gamal_public_key"]),
			crypto::cpp_int(-1)
			);
		_user_crypto_key_set.add_user_keys(user_id, user_crypto_keys);

		DEBUG_MSG("[Client::handle_receive_user_keys] Keys received successfully for user: " + std::to_string(user_id));
	} else {
		ERROR_MSG("[Client::handle_receive_user_keys] Failed to receive keys for user");
		DEBUG_MSG("[Client::handle_receive_user_keys] Response: " + response.dump());
	}
}

common::User Client::get_user() {
	return _user;
}

bool Client::is_registered() const noexcept {
	return _user.get_id() != 0;
}

bool Client::is_connected() {
	return _socket.is_open();
}

std::string Client::generate_random_string(const int& len) {
	std::random_device random_device;
	std::mt19937 generator(random_device());
	std::uniform_int_distribution<> distribution(0, alphabet.size() - 1);

	std::string str;

	for (std::size_t i = 0; i < len; ++i) {
		str += alphabet[distribution(generator)];
	}

	return str;
}

std::string Client::get_user_data_filename() const noexcept {
	return std::string(SOURCE_DIR) + "/client/user_data/" + "user_" + _user.get_nickname() + "_" + _server_address + "_" + std::to_string(_server_port) + ".json";
}

}