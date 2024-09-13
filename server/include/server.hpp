#pragma once

#include "debug.hpp"

#include <iostream>
#include <ostream>
#include <istream>
#include <ctime>
#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

using boost::asio::ip::tcp;

class Server : public boost::enable_shared_from_this<Server> {
public:

    Server(unsigned int port) : acceptor(io_service, tcp::endpoint(tcp::v4(), port)) {}
    ~Server();
    
    void start();

private:
    void start_request_handling();
    void handle_accept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code& error);
    void handle_request(boost::shared_ptr<tcp::socket> socket);

private:
    boost::asio::io_service io_service;
    tcp::acceptor acceptor;
    boost::shared_ptr<boost::thread> s_thread;
};