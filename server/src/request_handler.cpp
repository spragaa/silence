#include "request_handler.hpp"

namespace server {

using tcp = boost::asio::ip::tcp;

RequestHandler::RequestHandler(RepositoryManager& repo_manager,
                               ConnectedClientsManager& connected_clients_manager)
	: _repo_manager(repo_manager),
	_connected_clients_manager(connected_clients_manager) {
}

void RequestHandler::handle_request(boost::shared_ptr<tcp::socket> socket) {
	boost::asio::streambuf request_buf;
	boost::system::error_code error;

	while (true) {
		boost::asio::read_until(*socket, request_buf, "\r\n\r\n", error);

		if (error == boost::asio::error::eof) {
			DEBUG_MSG("[Server::handle_request] Client closed connection on socket: " + common::get_socket_info(*socket));

			_connected_clients_manager.remove_client_by_socket(socket);

			DEBUG_MSG("[Server::handle_authorize] Client removed from connected list: " + common::get_socket_info(*socket));
			DEBUG_MSG("[Server::handle_authorize] Currently, there are " + std::to_string(_connected_clients_manager.get_connected_count()) + " users connected");

			break;
		} else if (error) {
			ERROR_MSG(std::string(error.message()));
		}

		std::istream request_stream(&request_buf);
		std::string request_line;
		std::getline(request_stream, request_line);

		try {
			nlohmann::json request = nlohmann::json::parse(request_line);

			if (request.contains("type")) {
				if (request["type"] == "register") {
					handle_register(socket, request);
				} else if (request["type"] == "authorize") {
					handle_authorize(socket, request);
				} else if(request["type"] == "send_message") {
					handle_send_message(socket, request);
				} else if (request["type"] == "file_chunk") {
					handle_file_chunk(socket, request);
				} else if (request["type"] == "get_user_keys") {
					handle_get_user_keys(socket, request);
				} /* else if (request["type"] == "send_private_keys") {
					 handle_send_public_keys(socket, request);
					 } */
				else {
					nlohmann::json response = {
						{"status", "error"},
						{"response", "Unknown request type"}
					};
					boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"), error);
				}
			} else {
				nlohmann::json response = {
					{"status", "error"},
					{"response", "Missing request type"}
				};
				boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"), error);
			}
		} catch (const nlohmann::json::parse_error& e) {
			nlohmann::json response = {
				{"status", "error"},
				{"response", "Invalid JSON format"}
			};
			boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"), error);
		}

		if (error) {
			ERROR_MSG("[Server::handle_request] While sending response" + std::string(error.message()))
		}

		request_buf.consume(request_buf.size());
	}
}

void RequestHandler::handle_register(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
	DEBUG_MSG("[Server::handle_register] Received request: " + request.dump());
	std::string nickname = request["nickname"];
	std::string password = request["password"];
	std::string el_gamal_public_key = request["el_gamal_public_key"];
	std::string dsa_public_key = request["dsa_public_key"];

	common::User new_user(nickname, password);
	int user_id = _repo_manager.create_user(new_user);

	if (!_repo_manager.set_public_keys(user_id, el_gamal_public_key, dsa_public_key) ) {
		ERROR_MSG("[RequestHandler::handle_register] Set public keys failed for user " + std::to_string(user_id));
		// add retry logic
	}

	nlohmann::json response;

	if (user_id != 0) {
		response["status"] = "success";
		response["response"] =  "User registered successfully";
		response["user_id"] = user_id;
	} else {
		response["status"] = "error";
		response["response"] =  "Failed to register user";
	}

	DEBUG_MSG("[Server::handle_register] Sending response: " + response.dump());
	boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
}

void RequestHandler::handle_authorize(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
	std::string nickname = request["nickname"];
	std::string password = request["password"];
	int user_id = request["user_id"];

	bool auth_success = _repo_manager.authorize_user(user_id, nickname, password);
	nlohmann::json response;

	if (auth_success) {
		response["status"] = "success";
		response["response"] = "Authorization successful";

		_connected_clients_manager.add_client(user_id, socket);
		// "New user connected" in handle authorize may be counterintuitive?
		DEBUG_MSG("[Server::handle_authorize] New user connected, with id: " + std::to_string(user_id) + " on socket: " + common::get_socket_info(*socket));
		DEBUG_MSG("[Server::handle_authorize] Currently, there are " + std::to_string(_connected_clients_manager.get_connected_count()) + " users connected");
	} else {
		response["status"] = "error";
		response["response"] = "Authorization failed: Invalid credentials";
	}
	DEBUG_MSG("[Server::handle_authorize] Sending request: " + request.dump());
	boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
}

void RequestHandler::handle_send_message(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
	DEBUG_MSG("[Server::handle_send_message] Called on request: " + request.dump());

	int sender_id = request["sender_id"];
	std::string receiver_nickname = request["receiver_nickname"];
	std::string request_text = request["message_text"];

	nlohmann::json sender_response, receiver_response;

	int receiver_id = _repo_manager.get_user_id(receiver_nickname);
	if (receiver_id == 0) {
		sender_response["status"] = "error";
		sender_response["response"] = "Receiver not found";
		boost::asio::write(*socket, boost::asio::buffer(sender_response.dump() + "\r\n\r\n"));
		return;
	}

	common::Message new_msg(sender_id, receiver_id, request_text);
	int msg_metadata_id = _repo_manager.create_message(new_msg);

	if (msg_metadata_id) {
		new_msg.set_id(msg_metadata_id);
	} else {
		WARN_MSG("[Server::handle_send_message] msg_text_id and msg_metadata_id are not equal");
		// add more info?
		sender_response["status"] = "error";
		sender_response["response"] = "Failed to correctly save messages in database";
		boost::asio::write(*socket, boost::asio::buffer(sender_response.dump() + "\r\n\r\n"));
		return;
	}

	// we should not return failure here, some kind of retry logic or/and buffer is better
	// we also can do it async
	// if (msg_metadata_id == 0 || msg_text_id == 0) { // connection to redis is broken now :(
	if (msg_metadata_id == 0) {
		sender_response["status"] = "error";
		sender_response["response"] = "Failed to save message into db/s";
		boost::asio::write(*socket, boost::asio::buffer(sender_response.dump() + "\r\n\r\n"));
		return;
	}

	sender_response["status"] = "success";
	sender_response["response"] = "Message sent successfully";
	// sender_response["file_name"] = request_filename;
	sender_response.update(new_msg.to_json());
	DEBUG_MSG("[Server::handle_send_message] Response for sender: " + sender_response.dump());
	boost::asio::write(*socket, boost::asio::buffer(sender_response.dump() + "\r\n\r\n"));

	auto receiver_socket = _connected_clients_manager.get_client_socket(receiver_id);
	if (receiver_socket) {
		receiver_response.update(new_msg.to_json());
		receiver_response["type"] = "receive_msg";
		// receiver_response["file_name"] = request_filename;
		DEBUG_MSG("[Server::handle_send_message] Response for receiver: " + receiver_response.dump());
		boost::asio::write(*receiver_socket, boost::asio::buffer(receiver_response.dump() + "\r\n\r\n"));
	}

	if (request.contains("file_name") && request["file_name"] != "none") {
		std::string filename = request["file_name"];

		{
			std::lock_guard<std::mutex> lock(_pending_transfers_mutex);
			_pending_file_transfers.push_back(PendingFileTransfer{
					filename,
					sender_id,
					receiver_id
				});
			DEBUG_MSG("[Server::handle_send_message] Created pending transfer for file: " +
			          filename + " from user " + std::to_string(sender_id) +
			          " to user " + std::to_string(receiver_id));
		}

		_file_uploads[filename] = {0, false};

		sender_response["file_name"] = filename;
		DEBUG_MSG("[Server::handle_send_message] File transfer initialized for: " + filename);
	}

	sender_response["status"] = "success";
	sender_response["response"] = "Message sent successfully";
	sender_response.update(new_msg.to_json());
	DEBUG_MSG("[Server::handle_send_message] Response for sender: " + sender_response.dump());
	boost::asio::write(*socket, boost::asio::buffer(sender_response.dump() + "\r\n\r\n"));

	if (receiver_socket) {
		receiver_response.update(new_msg.to_json());
		receiver_response["type"] = "receive_msg";
		if (request.contains("file_name") && request["file_name"] != "none") {
			receiver_response["file_name"] = request["file_name"];
		}
		DEBUG_MSG("[Server::handle_send_message] Response for receiver: " + receiver_response.dump());
		boost::asio::write(*receiver_socket, boost::asio::buffer(receiver_response.dump() + "\r\n\r\n"));
	}
}

void RequestHandler::handle_file_chunk(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
	DEBUG_MSG("[Server::handle_file_chunk] Request: " + request.dump());

	std::string filename = request["filename"];
	std::string chunk_data = request["chunk_data"];
	size_t chunk_number = request["chunk_number"];
	bool is_last = request["is_last"];

	if (_file_uploads.find(filename) == _file_uploads.end()) {
		_file_uploads[filename] = {0, false};
	}

	auto& upload_state = _file_uploads[filename];

	nlohmann::json sender_response;
	sender_response["type"] = "chunk_acknowledgment";
	sender_response["filename"] = filename;
	sender_response["chunk_number"] = chunk_number;

	if (chunk_number != upload_state.last_chunk_received + 1) {
		WARN_MSG("[Server::handle_file_chunk] Received out-of-order chunk. Expected: "
		         + std::to_string(upload_state.last_chunk_received + 1)
		         + ", Got: " + std::to_string(chunk_number));
		sender_response["status"] = "error";
		sender_response["error"] = "wrong_chunk_order";
	}
	else if (_repo_manager.upload_file_chunk(filename, chunk_data)) {
		INFO_MSG("[Server::handle_file_chunk] Chunk " + std::to_string(chunk_number) + " uploaded");
		sender_response["status"] = "success";
		upload_state.last_chunk_received = chunk_number;

		if (is_last) {
			upload_state.completed = true;
			INFO_MSG("[Server::handle_file_chunk] File upload completed: " + filename);

			std::lock_guard<std::mutex> lock(_pending_transfers_mutex);
			auto pending_it = std::find_if(_pending_file_transfers.begin(),
			                               _pending_file_transfers.end(),
			                               [&filename](const PendingFileTransfer& transfer) {
					return transfer.filename == filename;
				});

			if (pending_it != _pending_file_transfers.end()) {
				auto receiver_socket = _connected_clients_manager.get_client_socket(pending_it->receiver_id);
				if (receiver_socket) {
					try {
						nlohmann::json file_transfer_notification;
						file_transfer_notification["type"] = "incoming_file";
						file_transfer_notification["filename"] = filename;
						file_transfer_notification["sender_id"] = pending_it->sender_id;

						DEBUG_MSG("[Server::handle_file_chunk] Sending file transfer notification: "
						          + file_transfer_notification.dump());

						boost::asio::write(*receiver_socket,
						                   boost::asio::buffer(file_transfer_notification.dump() + "\r\n\r\n"));

						send_file_to_client(receiver_socket, filename);
						_pending_file_transfers.erase(pending_it);

						INFO_MSG("[Server::handle_file_chunk] File " + filename
						         + " successfully transferred to receiver "
						         + std::to_string(pending_it->receiver_id));
					}
					catch (const std::exception& e) {
						ERROR_MSG("[Server::handle_file_chunk] Failed to send file to receiver: "
						          + std::string(e.what()));
					}
				} else {
					DEBUG_MSG("[Server::handle_file_chunk] Receiver "
					          + std::to_string(pending_it->receiver_id)
					          + " is offline. Transfer remains pending.");
				}
			} else {
				WARN_MSG("[Server::handle_file_chunk] No pending transfer found for file: "
				         + filename);
			}
		}
	} else {
		WARN_MSG("[Server::handle_file_chunk] Failed to upload chunk to file server");
		sender_response["status"] = "error";
		sender_response["error"] = "upload_failed";
	}

	try {
		boost::asio::write(*socket, boost::asio::buffer(sender_response.dump() + "\r\n\r\n"));
		DEBUG_MSG("[Server::handle_file_chunk] Sent acknowledgment: " + sender_response.dump());
	}
	catch (const std::exception& e) {
		ERROR_MSG("[Server::handle_file_chunk] Failed to send acknowledgment: "
		          + std::string(e.what()));
	}
}

void RequestHandler::handle_get_user_keys(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
	DEBUG_MSG("[Server::handle_get_user_keys] Received request: " + request.dump());
	std::string nickname = request["nickname"];

	nlohmann::json response;

	int receiver_id = _repo_manager.get_user_id(nickname);
	if (receiver_id == 0) {
		response["status"] = "error";
		response["response"] = "Receiver not found";
		boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
		return;
	}

	std::optional<common::crypto::UserCryptoKeys> receiver_crypto_keys = _repo_manager.get_public_keys(receiver_id);

	if (!receiver_crypto_keys.has_value()) {
		ERROR_MSG("[RequestHandler::handle_get_user_keys] Failed to retrieve keys for user: " + std::to_string(receiver_id));
		response["status"] = "error";
		response["response"] = "Failed to retrieve keys";
		boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));

		return;
		// add retry logic ???
	} else {
		response["status"] = "success";
		response["user_id"] = receiver_id;
		response["dsa_public_key"] = common::crypto::cpp_int_to_hex(receiver_crypto_keys->get_dsa_public_key());
		response["el_gamal_public_key"] = common::crypto::cpp_int_to_hex(receiver_crypto_keys->get_el_gamal_public_key());
	}

	DEBUG_MSG("[Server::handle_get_user_keys] Sending response: " + response.dump());
	boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
}

// I thought we will need this now, but actually it is better to handle this on register step
// then when keys cycle is over we will use these methods;
// void RequestHandler::handle_receive_public_keys(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
// }

// I thought we will need this now, but actually it is better to handle this on register step
// then when keys cycle is over we will use these methods;
// void RequestHandler::handle_send_public_keys(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
// }

void RequestHandler::send_file_to_client(boost::shared_ptr<tcp::socket> client_socket, const std::string& filename) {
	std::vector<std::string> chunks = _repo_manager.download_file_chunks(filename);

	if (chunks.empty()) {
		ERROR_MSG("[Server::send_file_to_client] Failed to download chunks for file: " + filename);
		return;
	}

	for (size_t i = 0; i < chunks.size(); ++i) {
		nlohmann::json chunk_message;
		chunk_message["type"] = "file_chunk";
		chunk_message["filename"] = filename;
		chunk_message["chunk_number"] = i;
		chunk_message["chunk_data"] = chunks[i];
		chunk_message["is_last"] = (i == chunks.size() - 1);

		try {
			boost::asio::write(*client_socket,
			                   boost::asio::buffer(chunk_message.dump() + "\r\n\r\n"));

			DEBUG_MSG("[Server::send_file_to_client] Sent chunk "
			          + std::to_string(i + 1) + "/" + std::to_string(chunks.size())
			          + " of file " + filename);
		}
		catch (const std::exception& e) {
			ERROR_MSG("[Server::send_file_to_client] Failed to send chunk: "
			          + std::string(e.what()));
			return;
		}
	}

	INFO_MSG("[Server::send_file_to_client] Successfully sent file " + filename
	         + " (" + std::to_string(chunks.size()) + " chunks)");
}

}