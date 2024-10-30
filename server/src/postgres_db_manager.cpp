#include "postgres_db_manager.hpp"

namespace server {

void PostgresDBManager::add_connection(const std::string& name, const std::string& connection_string) {
	_connections[name] = std::make_unique<pqxx::connection>(connection_string);
	DEBUG_MSG("Connected to db via connection string: " + connection_string);
}

pqxx::connection& PostgresDBManager::get_connection(const std::string& name) {
	auto it = _connections.find(name);
	if (it == _connections.end()) {
		FATAL_MSG("[PostgresDBManager::get_connection()] Database connection not found " + name);
	}
	return *(it->second);
}

}