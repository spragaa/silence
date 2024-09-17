#include "server.hpp"

Server::Server(unsigned short port, unsigned int thread_pool_size)
	: acceptor(io_service, tcp::endpoint(tcp::v4(), port)),
	work(new boost::asio::io_service::work(io_service)) {
	for (unsigned int i = 0; i < thread_pool_size; ++i) {
		thread_pool.push_back(boost::make_shared<boost::thread>(boost::bind(&boost::
		                                                                    asio::
		                                                                    io_service
		                                                                    ::run,
		                                                                    &
		                                                                    io_service)));
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

	boost::shared_ptr<tcp::socket> socket = boost::make_shared<tcp::socket>(
		io_service);

	acceptor.async_accept(*socket,
	                      boost::bind(&Server::handle_accept, this, socket,
	                                  boost::asio::placeholders::error));
}

void Server::handle_accept(boost::shared_ptr<tcp::socket>   socket,
                           const boost::system::error_code& error) {
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

        try {
            nlohmann::json request = nlohmann::json::parse(request_line);
            
            if (request.contains("type")) {
                if (request["type"] == "register") {
                    handle_register(socket, request);
                } else if (request["type"] == "authorize") {
                    handle_authorize(socket, request);
                } else {
                    nlohmann::json response = {
                        {"status", "error"},
                        {"message", "Unknown request type"}
                    };
                    boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"), error);
                }
            } else {
                nlohmann::json response = {
                    {"status", "error"},
                    {"message", "Missing request type"}
                };
                boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"), error);
            }
        } catch (const nlohmann::json::parse_error& e) {
            nlohmann::json response = {
                {"status", "error"},
                {"message", "Invalid JSON format"}
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
    
    auto it = std::find_if(users.begin(), users.end(),
        [&nickname](const auto& pair) { return pair.second.get_nickname() == nickname; });
    
    if (it != users.end()) {
        nlohmann::json response = {
            {"status", "error"},
            {"message", "User already exists"}
        };
        boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
    } else {
        User new_user(nickname, password);
        int user_id = new_user.get_id();
        users[user_id] = new_user;
        nlohmann::json response = {
            {"status", "success"},
            {"message", "User registered successfully"},
            {"user_id", user_id}
        };
        print_users();
        boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
    }
}

void Server::handle_authorize(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request) {
    std::string nickname = request["nickname"];
    std::string password = request["password"];
    int user_id = request["user_id"].get<int>();
    
    auto user_it = users.find(user_id);
    if (user_it != users.end() && user_it->second.get_nickname() == nickname && user_it->second.check_password(password)) {
        nlohmann::json response = {
            {"status", "success"},
            {"message", "User authorized successfully"},
        };
        boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
    } else {
        nlohmann::json response = {
            {"status", "error"},
            {"message", "Invalid credentials"}
        };
        boost::asio::write(*socket, boost::asio::buffer(response.dump() + "\r\n\r\n"));
    }
}

void inline Server::print_users() const noexcept {
    std::cout << "There are " << users.size() << " users registered in the chat application right now" << std::endl;
    std::cout << "id \t\t" << "nickname \t\t" << "password \t\t\n";
    
    for (const auto& [id, user] : users) {
        std::cout << id << "\t\t";
        std::cout << user.get_nickname() << "\t\t";
        std::cout << user.get_password() << "\t\t\n";
    }
}