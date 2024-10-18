#pragma once

#include "debug.hpp"
#include "user.hpp"
#include "message.hpp"
#include "postgres_db_manager.hpp"
#include "user_metadata_repository.hpp"
#include "message_metadata_repository.hpp"
#include "message_text_repository.hpp"

#include <iostream>
#include <ostream>
#include <iomanip>
#include <istream>
#include <chrono>
#include <ctime>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <memory>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <pqxx/except>

using boost::asio::ip::tcp;
using namespace boost::placeholders;

class Server : public boost::enable_shared_from_this<Server> {
public:
	Server(unsigned short port,
	       unsigned int thread_pool_size,
	       const std::string& user_metadata_db_connection_string, // add metadata to name
	       const std::string& msg_metadata_db_connection_string, // add metadata to name
	       const std::string& msg_text_db_connection_string
	       );
	~Server();

	void start();

private:
	void start_request_handling();
	// should I move these into RequestHandlerClas?
	void handle_accept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code& error);
	void handle_request(boost::shared_ptr<tcp::socket> socket);
	void handle_register(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	void handle_authorize(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	void handle_send_message(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);

	boost::asio::io_service _io_service;
	tcp::acceptor _acceptor;
	boost::shared_ptr<boost::asio::io_service::work> _work;

	PostgresDBManager _postgres_db_manager;
	std::unique_ptr<UserMetadataRepository> _user_repo;
	std::unique_ptr<MessageMetadataRepository> _msg_metadata_repo;
	std::unique_ptr<MessageTextRepository> _msg_text_repo;

	std::map<int, boost::shared_ptr<tcp::socket> > _connected_clients;
};