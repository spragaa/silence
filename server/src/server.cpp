#include "server.hpp"
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>

Server::Server(const ServerConfig& config)
	: _config(config),
	  _repo_manager(config),
      _acceptor(_io_service, tcp::endpoint(tcp::v4(), _config._port)),
	  _work(new boost::asio::io_service::work(_io_service)) {

	for (unsigned int i = 0; i < _config._thread_pool_size; ++i) {
		_thread_pool.push_back(boost::make_shared<boost::thread>(
								  boost::bind(&boost::asio::io_service::run, &_io_service)));
	}
}



Server::~Server() {
	_io_service.stop();
	for (auto& thread : _thread_pool) {
		thread->join();
	}
}

void Server::start() {
	INFO_MSG("Server started");
	start_request_handling();

	while (true) {
		try {
			_io_service.run();
		} catch (const std::exception& e) {
			FATAL_MSG("[Server::start()] " + std::string(e.what()));
			_io_service.reset();
		}
	}
}

void Server::start_request_handling() {
	DEBUG_MSG("[Server::start_request_handling] Start request handling");
	boost::shared_ptr<tcp::socket> socket = boost::make_shared<tcp::socket>(_io_service);
	_acceptor.async_accept(*socket,
	                       boost::bind(&Server::handle_accept, this, socket, boost::asio::placeholders::error));
}

void Server::handle_accept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code& error) {
	DEBUG_MSG("[Server::handle_accept] Called on a socket: " + get_socket_info(*socket));
	if (!error) {
		boost::asio::post(_io_service, [this, socket]() {
			handle_request(socket);
		});
		start_request_handling();
	}
}

void Server::handle_request(boost::shared_ptr<tcp::socket> socket) {
	boost::asio::streambuf request_buf;
	boost::system::error_code error;

	while (true) {
		boost::asio::read_until(*socket, request_buf, "\r\n\r\n", error);

		if (error == boost::asio::error::eof) {
			DEBUG_MSG("[Server::handle_request] Client closed connection on socket: " + get_socket_info(*socket));
			
			_connected_clients_manager.remove_client_by_socket(socket);
			
			// for(auto it = _connected_clients.begin(); it != _connected_clients.end(); ++it) {
			// 	if(it->second == socket) {
			// 		_connected_clients.erase(it);
			// 		break;
			// 	}
			// }
			DEBUG_MSG("[Server::handle_authorize] Client removed from connected list: " + get_socket_info(*socket));
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
				}
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

void Server::handle_register(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
	DEBUG_MSG("[Server::handle_register] Received request: " + request.dump());
	std::string nickname = request["nickname"];
	std::string password = request["password"];

	User new_user(nickname, password);
	int user_id = _repo_manager.create_user(new_user);
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

void Server::handle_authorize(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
	std::string nickname = request["nickname"];
	std::string password = request["password"];
	int user_id = request["user_id"];

	bool auth_success = _repo_manager.authorize_user(user_id, nickname, password);
	nlohmann::json response;

	if (auth_success) {
		response["status"] = "success";
		response["response"] = "Authorization successful";
		// User user = _user_repo.get_user(user_id);
		// response["user_data"] = {
		//     {"last_online_timestamp", std::chrono::system_clock::to_time_t(user.get_last_online_timestamp())},
		//     {"is_online", true}
		// };
		// _user_repo.update_user_status(user_id, true);
		_connected_clients_manager.add_client(user_id, socket);
		// _connected_clients[user_id] = socket;
		// "New user connected" in handle authorize may be counterintuitive?
		DEBUG_MSG("[Server::handle_authorize] New user connected, with id: " + std::to_string(user_id) + " on socket: " + get_socket_info(*socket));
		DEBUG_MSG("[Server::handle_authorize] Currently, there are " + std::to_string(_connected_clients_manager.get_connected_count()) + " users connected");
	} else {
		response["status"] = "error";
		response["response"] = "Authorization failed: Invalid credentials";
	}
	DEBUG_MSG("[Server::handle_authorize] Sending request: " + request.dump());
	boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
}

void Server::handle_send_message(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
	DEBUG_MSG("[Server::handle_send_message] Called on request: " + request.dump());

	int sender_id = request["sender_id"];
	std::string receiver_nickname = request["receiver_nickname"];
	std::string request_text = request["message_text"];
	// std::string request_filename = request["filename"];

	nlohmann::json sender_response, receiver_response;

	int receiver_id = _repo_manager.get_user_id(receiver_nickname);
	if (receiver_id == 0) {
		sender_response["status"] = "error";
		sender_response["response"] = "Receiver not found";
		boost::asio::write(*socket, boost::asio::buffer(sender_response.dump() + "\r\n\r\n"));
		return;
	}

	Message new_msg(sender_id, receiver_id, request_text);

	int msg_metadata_id = _repo_manager.create_message(new_msg);
	// [MessageTextRepository::create] unknown error code: Failed to connect to Redis: tu
	// int msg_text_id = _msg_text_repo->create(new_msg.get_text());
	// if (msg_metadata_id == msg_text_id) {
	if (msg_metadata_id) {
		new_msg.set_id(msg_metadata_id);
	} else {
		WARN_MSG("[Server::handle_send_message] msg_text_id and msg_metadata_id are not equal");
		// add more info, msg id ...
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
	sender_response.update(new_msg.to_json());
	DEBUG_MSG("[Server::handle_send_message] Response for sender: " + sender_response.dump());
	boost::asio::write(*socket, boost::asio::buffer(sender_response.dump() + "\r\n\r\n"));

	auto receiver_socket = _connected_clients_manager.get_client_socket(receiver_id);
    if (receiver_socket) {
        receiver_response.update(new_msg.to_json());
        receiver_response["type"] = "receive_msg";
        DEBUG_MSG("[Server::handle_send_message] Response for receiver: " + receiver_response.dump());
        boost::asio::write(*receiver_socket, boost::asio::buffer(receiver_response.dump() + "\r\n\r\n"));
    }
	
	if (request.contains("file_name") && request["file_name"] != "none") {
        std::string filename = request["file_name"];
        
        PendingFileTransfer transfer{
            filename,
            sender_id,
            receiver_id
        };
        
        {
            std::lock_guard<std::mutex> lock(_pending_transfers_mutex);
            _pending_file_transfers.push_back(transfer);
        }
        
        auto upload_it = _file_uploads.find(filename);
        if (upload_it != _file_uploads.end() && upload_it->second.completed) {
            auto receiver_socket = _connected_clients_manager.get_client_socket(receiver_id);
            if (receiver_socket) {
                nlohmann::json file_transfer_notification;
                file_transfer_notification["type"] = "incoming_file";
                file_transfer_notification["filename"] = filename;
                file_transfer_notification["sender_id"] = sender_id;
                
                boost::asio::write(*receiver_socket, 
                    boost::asio::buffer(file_transfer_notification.dump() + "\r\n\r\n"));
                
                send_file_to_client(receiver_socket, filename);
            }
        }
    }
}

void Server::handle_file_chunk(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
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

void Server::send_file_to_client(boost::shared_ptr<tcp::socket> client_socket, const std::string& filename) {
    std::vector<std::string> chunks = _repo_manager.download_file_chunks(filename);
    
    if (chunks.empty()) {
        ERROR_MSG("[Server::send_file_to_client] Failed to download chunks for file: " + filename);
        return;
    }
    
    for (size_t i = 0; i < chunks.size(); ++i) {
        nlohmann::json chunk_message;
        chunk_message["type"] = "file_chunk";
        chunk_message["filename"] = filename;
        chunk_message["chunk_number"] = i + 1;
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
