#pragma once 

#include <string>
#include <iostream>
#include <boost/asio.hpp>

std::string get_current_timestamp();
std::string get_socket_info(boost::asio::ip::tcp::socket& socket);

#define DEBUG_MSG(msg) std::cout << "[DEBUG] [" << get_current_timestamp() << "] " << msg << std::endl;

using Timestamp = std::chrono::time_point<std::chrono::system_clock>;
