#include "server.hpp"
#include <boost/bind.hpp>

namespace server {

using tcp = boost::asio::ip::tcp;

Server::Server(const ServerConfig& config)
	: _config(config),
	_repo_manager(config),
	_io_service(),
	_acceptor(_io_service),
	_work(new boost::asio::io_service::work(_io_service)),
	_request_handler(_repo_manager, _connected_clients_manager) {

	try {
		tcp::endpoint endpoint(tcp::v4(), _config._port);

		_acceptor.open(tcp::v4());

		boost::system::error_code ec;
		_acceptor.set_option(tcp::acceptor::reuse_address(true), ec);
		if (ec) {
			throw boost::system::system_error(ec, "Failed to set acceptor reuse_address option");
		}

		_acceptor.bind(endpoint);
		_acceptor.listen();

		for (unsigned int i = 0; i < _config._thread_pool_size; ++i) {
			_thread_pool.push_back(boost::make_shared<boost::thread>(
									   boost::bind(&boost::asio::io_service::run, &_io_service)));
		}

		INFO_MSG("Server initialized successfully on port " + std::to_string(_config._port));
	}
	catch (const boost::system::system_error& e) {
		FATAL_MSG("Failed to initialize server: " + std::string(e.what()));
		throw;
	}
}

Server::~Server() {
	try {
		if (_acceptor.is_open()) {
			boost::system::error_code ec;
			_acceptor.close(ec);
			if (ec) {
				ERROR_MSG("Error closing acceptor: " + ec.message());
			}
		}

		_work.reset();
		_io_service.stop();

		for (auto& thread : _thread_pool) {
			if (thread && thread->joinable()) {
				thread->join();
			}
		}

		INFO_MSG("Server shutdown completed");
	}
	catch (const std::exception& e) {
		ERROR_MSG("Error during server cleanup: " + std::string(e.what()));
	}
}

void Server::start() {
	try {
		INFO_MSG("Server starting on port " + std::to_string(_config._port));
		if (!_acceptor.is_open()) {
			ERROR_MSG("Acceptor is not open!");
			return;
		}
		start_request_handling();

		while (true) {
			try {
				_io_service.run();
			} catch (const std::exception& e) {
				FATAL_MSG("[Server::start()] " + std::string(e.what()));
				_io_service.reset();
			}
		}
	}
	catch (const std::exception& e) {
		FATAL_MSG("Failed to start server: " + std::string(e.what()));
		throw;
	}
}

void Server::start_request_handling() {
	try {
		DEBUG_MSG("[Server::start_request_handling] Start request handling");

		boost::shared_ptr<tcp::socket> socket = boost::make_shared<tcp::socket>(_io_service);

		_acceptor.async_accept(*socket,
		                       [this, socket](const boost::system::error_code& error) {
			if (!error) {
				try {
					boost::system::error_code ec;
					socket->set_option(tcp::socket::reuse_address(true), ec);
					if (ec) {
						ERROR_MSG("Failed to set socket options: " + ec.message());
					}

					socket->set_option(boost::asio::socket_base::keep_alive(true), ec);
					if (ec) {
						ERROR_MSG("Failed to set keep-alive option: " + ec.message());
					}

					boost::asio::post(_io_service, [this, socket]() {
						_request_handler.handle_request(socket);
					});
				}
				catch (const std::exception& e) {
					ERROR_MSG("Error handling accepted connection: " + std::string(e.what()));
				}
			} else {
				ERROR_MSG("Accept error: " + error.message());
			}

			start_request_handling();
		});
	}
	catch (const std::exception& e) {
		ERROR_MSG("Error in start_request_handling: " + std::string(e.what()));

		boost::shared_ptr<boost::asio::deadline_timer> timer =
			boost::make_shared<boost::asio::deadline_timer>(_io_service, boost::posix_time::seconds(1));
		timer->async_wait([this](const boost::system::error_code&) {
			start_request_handling();
		});
	}
}

void Server::handle_accept(boost::shared_ptr<tcp::socket> socket,
                           const boost::system::error_code& error) {
	DEBUG_MSG("[Server::handle_accept] Called on a socket: " + common::get_socket_info(*socket));
	if (!error) {
		boost::asio::post(_io_service, [this, socket]() {
			_request_handler.handle_request(socket);
		});
		start_request_handling();
	}
}

}