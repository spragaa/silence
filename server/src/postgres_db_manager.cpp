#include "postgres_db_manager.hpp"

void PostgresDBManager::add_connection(const std::string& name, const std::string& connection_string) {
	connections[name] = std::make_unique<pqxx::connection>(connection_string);
	DEBUG_MSG("Connected to db via connection string: " + connection_string);
}

pqxx::connection& PostgresDBManager::get_connection(const std::string& name) {
	auto it = connections.find(name);
	if (it == connections.end()) {
		FATAL_MSG("[PostgresDBManager::get_connection()] Database connection not found " + name);
	}
	return *(it->second);
}