#pragma once

#include "debug.hpp"
#include "user.hpp"
#include "message.hpp"
#include "repository_manager.hpp"
#include "connected_clients_manager.hpp"
#include "server_config.hpp"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <nlohmann/json.hpp>
#include <mutex>
#include <map>
#include <vector>

namespace server {

class RequestHandler {
	using tcp = boost::asio::ip::tcp;

public:
	RequestHandler(RepositoryManager& repo_manager,
	               ConnectedClientsManager& connected_clients_manager);

	void handle_request(boost::shared_ptr<tcp::socket> socket);

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

	void handle_register(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	void handle_authorize(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	void handle_send_message(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	void handle_file_chunk(boost::shared_ptr<tcp::socket> socket, const nlohmann::json& request);
	void send_file_to_client(boost::shared_ptr<tcp::socket> client_socket, const std::string& filename);

private:
	RepositoryManager& _repo_manager;
	ConnectedClientsManager& _connected_clients_manager;
	std::vector<PendingFileTransfer> _pending_file_transfers;
	std::mutex _pending_transfers_mutex;
	std::map<std::string, UploadState> _file_uploads;
};

}