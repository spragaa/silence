#include "debug.hpp"

#include <iomanip>
#include <chrono>
#include <ctime>

std::string get_current_timestamp() {
	auto now        = std::chrono::system_clock::now();
	auto time_t_now = std::chrono::system_clock::to_time_t(now);
	std::tm tm_now  = *std::localtime(&time_t_now);

	std::stringstream ss;

	ss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
	return ss.str();
}

std::string get_socket_info(boost::asio::ip::tcp::socket& socket) {
	std::ostringstream info;

	try {
		boost::asio::ip::tcp::endpoint local_ep = socket.local_endpoint();
		info << "Local IP: " << local_ep.address().to_string()
		     << ", Local Port: " << local_ep.port() << " ";

		boost::asio::ip::tcp::endpoint remote_ep = socket.remote_endpoint();
		info << "Remote IP: " << remote_ep.address().to_string()
		     << ", Remote Port: " << remote_ep.port() << "\n";
	} catch (boost::system::system_error& e) {
		info << "Error: " << e.what() << "\n";
	}
	return info.str();
}
