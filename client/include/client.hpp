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
#include <filesystem>

namespace fs = std::filesystem;

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
	void send_file_chunks(const std::string& filepath);
	
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

	boost::asio::io_service _io_service;
	std::unique_ptr<boost::asio::io_service::work> _work;
	std::vector<std::thread> _io_threads;
	boost::asio::ip::tcp::socket _socket;
	boost::asio::streambuf _read_buffer;
	std::queue<std::string> _write_queue;
	std::string _server_address;
	unsigned short _server_port;
	User _user;
	std::vector<Message> _messages;
	bool _is_authorized;
	std::string _user_files_dir = std::string(SOURCE_DIR) + "/client/user_files";
};
