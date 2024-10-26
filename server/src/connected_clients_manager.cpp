#include "connected_clients_manager.hpp"

void ConnectedClientsManager::add_client(int user_id, boost::shared_ptr<tcp::socket> socket) {
	_connected_clients[user_id] = socket;
}

void ConnectedClientsManager::remove_client(int user_id) {
	_connected_clients.erase(user_id);
}

void ConnectedClientsManager::remove_client_by_socket(boost::shared_ptr<tcp::socket> socket) {
	for (auto it = _connected_clients.begin(); it != _connected_clients.end(); ++it) {
		if (it->second == socket) {
			_connected_clients.erase(it);
			break;
		}
	}
}

boost::shared_ptr<tcp::socket> ConnectedClientsManager::get_client_socket(int user_id) {
	auto it = _connected_clients.find(user_id);
	if (it != _connected_clients.end()) {
		return it->second;
	}
	return nullptr;
}

bool ConnectedClientsManager::is_client_connected(int user_id) const {
	return _connected_clients.find(user_id) != _connected_clients.end();
}

size_t ConnectedClientsManager::get_connected_count() const {
	return _connected_clients.size();
}