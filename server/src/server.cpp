#include "server.hpp"

Server::~Server() {
    if (s_thread) {
        s_thread->join();
    }
}

void Server::start() {
    DEBUG_MSG("server started");

    s_thread = boost::make_shared<boost::thread>(boost::bind(&boost::asio::io_service::run, &io_service));

    start_request_handling();
}

void Server::start_request_handling() {
    DEBUG_MSG("server request handling is called");

    boost::shared_ptr<tcp::socket> socket = boost::make_shared<tcp::socket>(io_service);

    acceptor.async_accept(*socket, 
        boost::bind(&Server::handle_accept, this, socket, boost::asio::placeholders::error));
}

void Server::handle_accept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code& error) {
    if (!error) {
        DEBUG_MSG("client connected");
        
        boost::thread(boost::bind(&Server::handle_request, this, socket));

        start_request_handling();
    }
}

void Server::handle_request(boost::shared_ptr<tcp::socket> socket) {
    boost::asio::streambuf request_buf;
    boost::system::error_code error;

    boost::asio::read_until(*socket, request_buf, "\r\n\r\n", error);

    if (!error) {
        std::istream request_stream(&request_buf);
        std::string request_line;
        std::getline(request_stream, request_line);

        DEBUG_MSG("Received request: " + request_line);

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 66\r\n"
            "Connection: close\r\n\r\n"
            "BLABLABALBALABLA";
        
        DEBUG_MSG(response);

        boost::asio::write(*socket, boost::asio::buffer(response), error);
    }

    socket->close();
}