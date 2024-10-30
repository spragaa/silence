#pragma once

#include <map>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

class ConnectedClientsManager {
    using tcp = boost::asio::ip::tcp;

public:
    ConnectedClientsManager() = default;
	~ConnectedClientsManager() = default;

	void add_client(int user_id, boost::shared_ptr<tcp::socket> socket);
	void remove_client(int user_id);
	void remove_client_by_socket(boost::shared_ptr<tcp::socket> socket);
	boost::shared_ptr<tcp::socket> get_client_socket(int user_id);
	bool is_client_connected(int user_id) const;
	size_t get_connected_count() const;

	std::map<int, boost::shared_ptr<tcp::socket> >::iterator begin() {
		return _connected_clients.begin();
	}
	std::map<int, boost::shared_ptr<tcp::socket> >::iterator end() {
		return _connected_clients.end();
	}
	std::map<int, boost::shared_ptr<tcp::socket> >::const_iterator begin() const {
		return _connected_clients.begin();
	}
	std::map<int, boost::shared_ptr<tcp::socket> >::const_iterator end() const {
		return _connected_clients.end();
	}

private:
	std::map<int, boost::shared_ptr<tcp::socket> > _connected_clients;
};