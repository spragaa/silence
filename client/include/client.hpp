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
#include <random>

namespace fs = std::filesystem;

class Client {
public:
	Client(const std::string& server_address,
	       unsigned short server_port, const std::string& nick);
	~Client();

	void run();
	User get_user();

private:
	struct FileTransferState;

	void        send_message(const std::string& message);
	void inline show_actions();
	std::string read_user_text() const noexcept;
	std::string receive_response();
	std::string get_user_data_filename() const noexcept;
	void send_file_chunks(const std::string& filepath);
	void send_next_chunk(std::shared_ptr<FileTransferState> state);
	void wait_for_chunk_ack(std::shared_ptr<FileTransferState> state);

	void handle_chunk_acknowledgment(const nlohmann::json& response);
	void handle_incoming_file(const nlohmann::json& notification);
	void handle_incoming_file_chunk(const nlohmann::json& chunk_message);

	bool is_registered() const noexcept;
	bool is_connected();
	void register_user();
	void authorize_user();
	void handle_user_interaction();
	std::string generate_random_string(const int& len);

	void async_read();
	void handle_async_read(const boost::system::error_code& error, size_t bytes_transferred);
	void async_write(const std::string& message);
	void handle_async_write(const boost::system::error_code& error);
	void do_write();
	void process_server_message(const std::string& message);

private:
	struct FileTransferState {
		std::ifstream file;
		size_t chunk_number = 0;
		std::vector<char> buffer;
		static const size_t chunk_size = 512;
		std::string filename;

		FileTransferState(const std::string& path) :
			file(path, std::ios::binary),
			buffer(chunk_size),
			filename(fs::path(path).filename().string()) {
		}
	};

	boost::asio::io_service _io_service;
	std::unique_ptr<boost::asio::io_service::work> _work;
	std::vector<std::thread> _io_threads;
	boost::asio::ip::tcp::socket _socket;
	boost::asio::streambuf _read_buffer;
	std::queue<std::string> _write_queue;
	std::condition_variable _chunk_cv;
	std::mutex _mutex;
	bool _chunk_acknowledged = false;
	std::string _server_address;
	unsigned short _server_port;
	User _user;
	std::vector<Message> _messages;
	bool _is_authorized;
	std::string _user_files_dir = std::string(SOURCE_DIR) + "/client/user_files";
	std::map<std::string, std::ofstream> _incoming_files;
};
