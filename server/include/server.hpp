#pragma once

#include "debug.hpp"
#include "user.hpp"
#include "message.hpp"
#include "db_manager.hpp"
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
	       const std::string& user_db_connection_string, // add metadata to name
	       const std::string& message_db_connection_string, // add metadata to name
	       const std::string& message_text_db_connection_string
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

	boost::asio::io_service io_service;
	tcp::acceptor acceptor;
	boost::shared_ptr<boost::asio::io_service::work> work;
	std::vector<boost::shared_ptr<boost::thread> > thread_pool;

	// ??? implement some kind of db manager, that will manage all 4 dbs
	// user_metadata, message_metadata - PostgresSQL
	// message_text - Redis
	// media_files - ???????
	//
	// rename DBManager to postgres db manager
	DBManager db_manager;
	// add metadata to name
	std::unique_ptr<UserMetadataRepository> user_repository;
	// add metadata to name
	std::unique_ptr<MessageMetadataRepository> message_repository;
	std::unique_ptr<MessageTextRepository> message_text_repository;
	
	std::set<int> connected_users;
};