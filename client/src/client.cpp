#include "client.hpp"

Client::Client(const std::string& server_address,
               unsigned short server_port,
               const std::string& nickname)
	: io_service()
	, socket(io_service)
	, server_address(server_address)
	, server_port(server_port)
	, user(nickname)
{
}

User Client::get_user() {
	return user;
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
			std::string message = get_user_input();

			if (message == "exit") {
				break;
			}

			send_message(message);
			std::string response = receive_response();
			std::cout << "Server response: " << response << std::endl;
		}

		socket.close();
	} catch (const std::exception& e) {
		std::cerr << "Exception in Client::run() : " << e.what() << std::endl;
	}
}

std::string Client::get_user_input() const noexcept{
	std::string message;

	std::cout << "Enter message to send (or 'exit' to quit): ";
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
