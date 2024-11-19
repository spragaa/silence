#pragma once

#include "common.hpp"

#include <string>
#include <iostream>
#include <boost/asio.hpp>

namespace common {

std::string get_current_timestamp();
std::string get_socket_info(boost::asio::ip::tcp::socket& socket);

} //namespace common

#define RED     "\x1b[31m"
#define ORANGE  "\x1b[38;5;208m"
#define GREY    "\x1b[90m"
#define RESET   "\x1b[0m"

#if DEBUG_LEVEL == 1
	#define DEBUG_MSG(msg) std::cout << GREY << "[DEBUG] [" << common::get_current_timestamp() << "] " << msg << RESET << std::endl
#else
	#define DEBUG_MSG(msg) do {} while (0)
#endif

#define INFO_MSG(msg) std::cout << "[INFO] [" << common::get_current_timestamp() << "] " << msg << std::endl;

#define WARN_MSG(msg) std::cout << ORANGE << "[WARN] [" << common::get_current_timestamp() << "] " << msg << RESET << std::endl;

#define ERROR_MSG(msg) std::cout << RED << "[ERROR] [" << common::get_current_timestamp() << "] " << msg << RESET << std::endl;

#define FATAL_MSG(msg) std::cout << RED << "[FATAL] [" << common::get_current_timestamp() << "] " << msg << RESET << std::endl;

// add USER_MSG?
// add spammy logs? full requests and aes rounds????
