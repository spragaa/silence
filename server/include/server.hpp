#pragma once

#include "debug.hpp"
#include "server_config.hpp"
#include "repository_manager.hpp"
#include "connected_clients_manager.hpp"
#include "request_handler.hpp"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <memory>

namespace server {

class Server : public boost::enable_shared_from_this<Server> {
	using tcp = boost::asio::ip::tcp;

public:
	Server(const ServerConfig& config);
	~Server();

	void start();

private:
	void start_request_handling();
	void handle_accept(boost::shared_ptr<tcp::socket> socket,
	                   const boost::system::error_code& error);

private:
	boost::asio::io_service _io_service;
	tcp::acceptor _acceptor;
	boost::shared_ptr<boost::asio::io_service::work> _work;
	std::vector<boost::shared_ptr<boost::thread> > _thread_pool;
	ServerConfig _config;
	RepositoryManager _repo_manager;
	ConnectedClientsManager _connected_clients_manager;
	RequestHandler _request_handler;
};

}