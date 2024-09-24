#include "server.hpp"
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <iostream>

Server::Server(unsigned short port, unsigned int thread_pool_size, 
               const std::string& user_db_connection_string,
               const std::string& message_db_connection_string)
    : acceptor(io_service, tcp::endpoint(tcp::v4(), port)),
      work(new boost::asio::io_service::work(io_service)) {
          
    db_manager.add_connection("user_db", user_db_connection_string);
    db_manager.add_connection("message_db", message_db_connection_string);
          
    user_repository = std::make_unique<UserRepository>(db_manager, "user_db");
    message_repository = std::make_unique<MessageRepository>(db_manager, "message_db");
          
    for (unsigned int i = 0; i < thread_pool_size; ++i) {
        thread_pool.push_back(boost::make_shared<boost::thread>(
            boost::bind(&boost::asio::io_service::run, &io_service)));
    }
}

Server::~Server() {
    io_service.stop();
    for (auto& thread : thread_pool) {
        thread->join();
    }
}

void Server::start() {
    std::cout << "Server started" << std::endl;
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
    boost::shared_ptr<tcp::socket> socket = boost::make_shared<tcp::socket>(io_service);
    acceptor.async_accept(*socket,
        boost::bind(&Server::handle_accept, this, socket, boost::asio::placeholders::error));
}

void Server::handle_accept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code& error) {
    if (!error) {
        boost::asio::post(io_service, [this, socket]() {
            handle_request(socket);
        });
        start_request_handling();
    }
}

void Server::handle_request(boost::shared_ptr<tcp::socket> socket) {
    boost::asio::streambuf request_buf;
    boost::system::error_code error;

    while (true) {
        boost::asio::read_until(*socket, request_buf, "\r\n\r\n", error);

        if (error == boost::asio::error::eof) {
            std::cout << "Client closed connection" << std::endl;
            break;
        } else if (error) {
            throw std::runtime_error("Error while receiving request: " + error.message());
        }

        std::istream request_stream(&request_buf);
        std::string request_line;
        std::getline(request_stream, request_line);

        try {
            nlohmann::json request = nlohmann::json::parse(request_line);
            
            if (request.contains("type")) {
                if (request["type"] == "register") {
                    handle_register(socket, request);
                } else if (request["type"] == "authorize") {
                    handle_authorize(socket, request);
                } else if(request["type"] == "send_message") {
                    handle_send_message(socket, request);
                } else {
                    nlohmann::json response = {
                        {"status", "error"},
                        {"response", "Unknown request type"}
                    };
                    boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"), error);
                }
            } else {
                nlohmann::json response = {
                    {"status", "error"},
                    {"response", "Missing request type"}
                };
                boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"), error);
            }
        } catch (const nlohmann::json::parse_error& e) {
            nlohmann::json response = {
                {"status", "error"},
                {"response", "Invalid JSON format"}
            };
            boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"), error);
        }

        if (error) {
            throw std::runtime_error("Error while sending response: " + error.message());
        }

        request_buf.consume(request_buf.size());
    }
}

void Server::handle_register(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
    std::string nickname = request["nickname"];
    std::string password = request["password"];
    
    User new_user(nickname, password);
    if (user_repository->create(new_user)) {
        nlohmann::json response = {
            {"status", "success"},
            {"response", "User registered successfully"},
            {"user_id", new_user.get_id()}
            // {"registered_timestamp", new_user.get_registered_timestamp()}
        };
        boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
    } else {
        nlohmann::json response = {
            {"status", "error"},
            {"response", "Failed to register user"}
        };
        boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
    }
    
}

void Server::handle_authorize(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
    std::string nickname = request["nickname"];
    std::string password = request["password"];
    
    auto user = user_repository->findByNickname(nickname);
    if (user && user->check_password(password)) {
        nlohmann::json response = {
            {"status", "success"},
            {"response", "User authorized successfully"},
            {"user_id", user->get_id()}
        };
        boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
    } else {
        nlohmann::json response = {
            {"status", "error"},
            {"response", "Invalid credentials"}
        };
        boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
    }
}

void Server::handle_send_message(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
    int sender_id = request["sender_id"];
    std::string receiver_nickname = request["receiver_nickname"];
    std::string message_text = request["message_text"];
    
    auto receiver = user_repository->findByNickname(receiver_nickname);
    if (!receiver) {
        nlohmann::json response = {
            {"status", "error"},
            {"response", "Receiver not found"}
        };
        boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
        return;
    }
    
    Message new_message(sender_id, receiver->get_id(), message_text);
    if (message_repository->create(new_message)) {
        nlohmann::json response = {
            {"status", "success"},
            {"response", "Message sent successfully"},
            {"message_id", new_message.get_id()}
        };
        boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
    } else {
        nlohmann::json response = {
            {"status", "error"},
            {"response", "Failed to send message"}
        };
        boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
    }
}