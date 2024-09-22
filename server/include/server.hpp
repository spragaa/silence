#pragma once

#include "debug.hpp"
#include "user.hpp"
#include "message.hpp"

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

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <pqxx/except>

using boost::asio::ip::tcp;

class Server : public boost::enable_shared_from_this<Server> {
public:

	Server(unsigned short port,
	       unsigned int thread_pool_size);
	~Server();

	void start();

private:

	void start_request_handling();
	void handle_accept(boost::shared_ptr<tcp::socket>   socket,
	                   const boost::system::error_code& error);
	void handle_request(boost::shared_ptr<tcp::socket>socket);
	void handle_register(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	void handle_authorize(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	void handle_send_message(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	
	void inline print_users() const noexcept; 
	void inline print_messages() const noexcept; 

private:

	boost::asio::io_service io_service;
	tcp::acceptor acceptor;
	boost::shared_ptr<boost::asio::io_service::work>work;
	std::vector<boost::shared_ptr<boost::thread> >thread_pool;
	
	pqxx::connection user_metadata_db_connection;
	std::unique_ptr<pqxx::work> user_metadata_db_worker;
	
	// remove after DB connection
	std::map<int, User> users;
	std::vector<Message> messages; // vector is stupid, but good for now
};
