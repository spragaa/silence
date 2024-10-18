#include "server.hpp"
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>

Server::Server(unsigned short port, unsigned int thread_pool_size,
               const std::string& user_metadata_db_connection_string,
               const std::string& msg_text_db_connection_string,
               const std::string& message_text_db_connection_string
               )
	: acceptor(io_service, tcp::endpoint(tcp::v4(), port)),
	work(new boost::asio::io_service::work(io_service)) {

	postgres_db_manager.add_connection("user_metadata_db", user_metadata_db_connection_string);
	postgres_db_manager.add_connection("message_metadata_db", msg_text_db_connection_string);

	user_repo = std::make_unique<UserMetadataRepository>(postgres_db_manager, "user_metadata_db");
	msg_metadata_repo = std::make_unique<MessageMetadataRepository>(postgres_db_manager, "message_metadata_db");
	msg_text_repo = std::make_unique<MessageTextRepository>(message_text_db_connection_string);
}

Server::~Server() {
	io_service.stop();
}

void Server::start() {
	INFO_MSG("Server started");
	start_request_handling();

	while (true) {
		try {
			io_service.run();
		} catch (const std::exception& e) {
			FATAL_MSG("[Server::start()] " + std::string(e.what()));
			io_service.reset();
		}
	}
}

void Server::start_request_handling() {
	DEBUG_MSG("[Server::start_request_handling] Start request handling");
	boost::shared_ptr<tcp::socket> socket = boost::make_shared<tcp::socket>(io_service);
	acceptor.async_accept(*socket,
	                      boost::bind(&Server::handle_accept, this, socket, boost::asio::placeholders::error));
}

void Server::handle_accept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code& error) {
	DEBUG_MSG("[Server::handle_accept] Called on a socket: " + get_socket_info(*socket));
	if (!error) {
		boost::asio::post(io_service, [this, socket]() {
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

			for(auto it = connected_clients.begin(); it != connected_clients.end(); ++it) {
				if(it->second == socket) {
					connected_clients.erase(it);
					break;
				}
			}
			DEBUG_MSG("[Server::handle_authorize] Client removed from connected list: " + get_socket_info(*socket));
			DEBUG_MSG("[Server::handle_authorize] Currently, there are " + std::to_string(connected_clients.size()) + " users connected");

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
				} else {
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
	int user_id = user_repo->create(new_user);
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

	bool auth_success = user_repo->authorize(user_id, nickname, password);
	nlohmann::json response;

	if (auth_success) {
		response["status"] = "success";
		response["response"] = "Authorization successful";
		// User user = user_repo.get_user(user_id);
		// response["user_data"] = {
		//     {"last_online_timestamp", std::chrono::system_clock::to_time_t(user.get_last_online_timestamp())},
		//     {"is_online", true}
		// };
		// user_repo.update_user_status(user_id, true);

		connected_clients[user_id] = socket;
		// "New user connected" in handle authorize may be counterintuitive?
		DEBUG_MSG("[Server::handle_authorize] New user connected, with id: " + std::to_string(user_id) + " on socket: " + get_socket_info(*socket));
		DEBUG_MSG("[Server::handle_authorize] Currently, there are " + std::to_string(connected_clients.size()) + " users connected");
	} else {
		response["status"] = "error";
		response["response"] = "Authorization failed: Invalid credentials";
	}
	DEBUG_MSG("[Server::handle_authorize] Sending request: " + request.dump());
	boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
}

void Server::handle_send_message(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
	int sender_id = request["sender_id"];
	std::string receiver_nickname = request["receiver_nickname"];
	std::string request_text = request["message_text"];
	
	nlohmann::json sender_response, receiver_response;

	int receiver_id = user_repo->get_id(receiver_nickname);
	if (receiver_id == 0) {
		sender_response["status"] = "error";
		sender_response["response"] = "Receiver not found";
		boost::asio::write(*socket, boost::asio::buffer(sender_response.dump() + "\r\n\r\n"));
		return;
	}

	// get id, how?
	Message new_msg(sender_id, receiver_id, request_text);

	// will they be same??? 
	int msg_metadata_id = msg_metadata_repo->create(new_msg.get_metadata());
	int msg_text_id = msg_text_repo->create(new_msg.get_text());
	
	if (msg_metadata_id == msg_text_id) {
	   new_msg.set_id(msg_text_id);
	} else {
        WARN_MSG("[Server::handle_send_message] msg_text_id and msg_metadata_id are not equal");
        sender_response["status"] = "error";
		sender_response["response"] = "Failed to correctly save messages in database";
		boost::asio::write(*socket, boost::asio::buffer(sender_response.dump() + "\r\n\r\n"));
		return;
	}
	
	// we should not return failure here, some kind of retry logic or/and buffer is better
	// we also can do it async
	if (msg_metadata_id == 0 || msg_text_id == 0) {
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

	for(auto it = connected_clients.begin(); it != connected_clients.end(); ++it) {
		if (it->first == receiver_id) {
			receiver_response.update(new_msg.to_json());
			receiver_response["type"] = "receive_msg";

			DEBUG_MSG("[Server::handle_send_message] Response for receiver: " + receiver_response.dump());
			boost::asio::write(*(it->second), boost::asio::buffer(receiver_response.dump() + "\r\n\r\n"));
		}
	}
}