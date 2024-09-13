#include <iostream>

#include "client.hpp"

#include <iostream>
#include <string>
#include <boost/asio.hpp>

int main() {
    try {
        boost::asio::io_service io_service;

        std::string server_address = "127.0.0.1";
        unsigned short server_port = 55555;

        boost::asio::ip::tcp::socket socket(io_service);

        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query(server_address, std::to_string(server_port));
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

        boost::asio::connect(socket, endpoint_iterator);

        std::string message;
        std::cout << "Enter message to send: ";
        std::getline(std::cin, message);

        boost::asio::write(socket, boost::asio::buffer(message + "\r\n\r\n"));

        boost::asio::streambuf response_buf;
        boost::asio::read_until(socket, response_buf, "\r\n\r\n");

        std::istream response_stream(&response_buf);
        std::string response;
        std::getline(response_stream, response);
        std::cout << "Server response: " << response << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}