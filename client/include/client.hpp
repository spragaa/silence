#pragma once

#include <iostream>
#include <string>
#include <boost/asio.hpp>

class Client {
public:
    Client(const std::string& server_address, unsigned short server_port);
    
    void run();

private:
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::socket socket;
    std::string server_address;
    unsigned short server_port;

    void send_message(const std::string& message);
    std::string get_user_input();
    std::string receive_response();
};
