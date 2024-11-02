#pragma once

#include "debug.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <filesystem>
#include <fstream>
#include <chrono>

namespace file_server {

class FileServerClient {
public:
	FileServerClient(const std::string& host, const std::string& port);

	// adjust all methods to return bool where possible
	// put server response messages into enum
	// legacy :)
	// std::string upload_file(const std::string& filename, const std::string& filepath);
	bool upload_chunk(const std::string& filename, const std::string& chunk_data);
	std::string download_file(const std::string& filename);
	std::string delete_file(const std::string& filename);
	std::vector<std::string> download_file_chunks(const std::string& filename);

private:
	std::string send_request(const std::string& target, boost::beast::http::verb method, const std::string& body = "");

private:
	std::string _host;
	std::string _port;
	const int _version = 11;
	boost::asio::io_context _io_context;
	boost::asio::ip::tcp::resolver _resolver;
	boost::beast::tcp_stream _stream;
};

}