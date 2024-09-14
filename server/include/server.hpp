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
    Server(unsigned short port, unsigned int thread_pool_size);
    ~Server();
    
    void start();

private:
    void start_request_handling();
    void handle_accept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code& error);
    void handle_request(boost::shared_ptr<tcp::socket> socket);

private:
    boost::asio::io_service io_service;
    tcp::acceptor acceptor;
    boost::shared_ptr<boost::asio::io_service::work> work;
    std::vector<boost::shared_ptr<boost::thread>> thread_pool;
};