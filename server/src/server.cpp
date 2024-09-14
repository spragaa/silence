#include "server.hpp"

Server::Server(unsigned short port, unsigned int thread_pool_size)
    : acceptor(io_service, tcp::endpoint(tcp::v4(), port)), 
      work(new boost::asio::io_service::work(io_service)) {
    for (unsigned int i = 0; i < thread_pool_size; ++i) {
        thread_pool.push_back(boost::make_shared<boost::thread>(boost::bind(&boost::asio::io_service::run, &io_service)));
    }
}

Server::~Server() {
    io_service.stop();
    for (auto& thread : thread_pool) {
        thread->join();
    }
}

void Server::start() {
    DEBUG_MSG("server started");

    start_request_handling();

    while (true) {
        try {
            io_service.run();
        } catch (const std::exception& e) {
            std::cerr << "Exception in Server::start(): " << e.what() << std::endl;
            io_service.reset();
        }
    }
}

void Server::start_request_handling() {
    DEBUG_MSG("server request handling is called");

    boost::shared_ptr<tcp::socket> socket = boost::make_shared<tcp::socket>(io_service);

    acceptor.async_accept(*socket, 
        boost::bind(&Server::handle_accept, this, socket, boost::asio::placeholders::error));
}

void Server::handle_accept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code& error) {
    if (!error) {
        DEBUG_MSG("handle_accept on socket: " + get_socket_info(*socket));

        boost::asio::post(io_service, [this, socket]() {
            handle_request(socket);
        });

        start_request_handling();
    }
}

void Server::handle_request(boost::shared_ptr<tcp::socket> socket) {
    DEBUG_MSG("handle_request on socket: " + get_socket_info(*socket));

    boost::asio::streambuf request_buf;
    boost::system::error_code error;

    while (true) {
        boost::asio::read_until(*socket, request_buf, "\r\n\r\n", error);

        if (error == boost::asio::error::eof) {
            DEBUG_MSG("Client closed connection on socket: " + get_socket_info(*socket));
            break;
        } else if (error) {
            throw std::runtime_error("Error while receiving request: " + error.message());
        }

        std::istream request_stream(&request_buf);
        std::string request_line;
        std::getline(request_stream, request_line);

        DEBUG_MSG("Received request: " + request_line);
        DEBUG_MSG("on socket: " + get_socket_info(*socket));

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 66\r\n"
            "Connection: keep-alive\r\n\r\n"
            "Server response: HTTP/1.1 200 OK";

        DEBUG_MSG(response);

        boost::asio::write(*socket, boost::asio::buffer(response), error);

        if (error) {
            throw std::runtime_error("Error while sending response: " + error.message());
        }

        request_buf.consume(request_buf.size());
    }
}