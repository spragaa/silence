#include "server.hpp"

Server::Server(unsigned int port, unsigned int thread_pool_size)
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
            std::cerr << "Exception: " << e.what() << std::endl;
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
        DEBUG_MSG("client connected");

        boost::asio::post(io_service, [this, socket]() {
            handle_request(socket);
        });

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
