#pragma once

#include "debug.hpp"
#include "user.hpp"
#include "message.hpp"

#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <nlohmann/json.hpp>

class Client {
public:

	Client(const std::string& server_address,
	       unsigned short server_port, const std::string& nick);
	~Client();

	void run();
	User get_user();

private:

	void        send_message(const std::string& message);
	void inline show_actions();
	std::string read_user_text() const noexcept;
	std::string receive_response();
	std::string get_user_data_filename() const noexcept;

	bool is_registered() const noexcept;
	void register_user();
	void authorize_user();
	void handle_user_interaction();

	void async_read();
	void handle_async_read(const boost::system::error_code& error, size_t bytes_transferred);
	void async_write(const std::string& message);
	void handle_async_write(const boost::system::error_code& error);
	void do_write();
	void process_server_message(const std::string& message);
	bool is_connected();

private:

	boost::asio::io_service io_service;
	std::unique_ptr<boost::asio::io_service::work> work;
	std::vector<std::thread> io_threads;
	boost::asio::ip::tcp::socket socket;
	boost::asio::streambuf read_buffer;
	std::queue<std::string> write_queue;
	std::string server_address;
	unsigned short server_port;
	User user;
	std::vector<Message> messages; // vector is stupid, but good for now
	bool is_authorized;
};
