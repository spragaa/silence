#pragma once

#include "debug.hpp"
#include "user.hpp"
#include "message.hpp"
#include "postgres_db_manager.hpp"
#include "user_metadata_repository.hpp"
#include "message_metadata_repository.hpp"
#include "message_text_repository.hpp"
#include "file_server_client.hpp"
#include "server_config.hpp"
#include "repository_manager.hpp"
#include "connected_clients_manager.hpp"

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
#include <queue>
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
	Server(const ServerConfig& config);
	~Server();

	void start();

private:
    struct UploadState;
    struct PendingFileTransfer;

	void start_request_handling();
	// should I move these into RequestHandlerClass?
	void handle_accept(boost::shared_ptr<tcp::socket> socket, const boost::system::error_code& error);
	void handle_request(boost::shared_ptr<tcp::socket> socket);

	// base class in handle request
	// each of these methods is a derived class 
	void handle_register(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	void handle_authorize(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	void handle_send_message(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	void handle_file_chunk(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);

	void send_file_to_client(boost::shared_ptr<tcp::socket> client_socket, const std::string& filename);
	
private:
    struct UploadState {
        size_t last_chunk_received;
        bool completed;
    };

    struct PendingFileTransfer {
        std::string filename;
        int sender_id;
        int receiver_id;
    };
    // queue?
    std::vector<PendingFileTransfer> _pending_file_transfers;
    std::mutex _pending_transfers_mutex;
    
    boost::asio::io_service _io_service;
	tcp::acceptor _acceptor;
	boost::shared_ptr<boost::asio::io_service::work> _work;
	std::vector<boost::shared_ptr<boost::thread> > _thread_pool;
	std::map<std::string, UploadState> _file_uploads;
	
	ServerConfig _config;
	RepositoryManager _repo_manager;
	ConnectedClientsManager _connected_clients_manager;
};