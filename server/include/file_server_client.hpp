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

namespace fs = std::filesystem;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class FileServerClient {
public:
    FileServerClient(const std::string& host, const std::string& port);
    
    std::string list_files();
    std::string upload_file(const std::string& filename, const std::string& filepath);
    std::string download_file(const std::string& filename);
    std::string delete_file(const std::string& filename);

private:
    std::string send_request(const std::string& target, http::verb method, const std::string& body = "");

private:
    std::string _host;
    std::string _port;
    const int _version = 11;
    net::io_context _io_context;
    tcp::resolver _resolver;
    beast::tcp_stream _stream;
};