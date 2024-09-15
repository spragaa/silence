#pragma once

#include "debug.hpp"
#include "user.hpp"

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

class Client {
public:

	Client(const std::string& server_address,
	       unsigned short server_port,
	       const std::string& nickname);

	void run();
	User get_user();

private:

	void        send_message(const std::string& message);
    void inline show_actions();
    std::string read_user_text() const noexcept;
	std::string receive_response();

private:

	boost::asio::io_service io_service;
	boost::asio::ip::tcp::socket socket;
	std::string server_address;
	unsigned short server_port;
	User user;
};
