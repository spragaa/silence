#include "repository_manager.hpp"
#include "debug.hpp"

RepositoryManager::RepositoryManager(const ServerConfig& config) {
	_postgres_db_manager.add_connection("user_metadata_db", config._user_metadata_db_connection_string);
	_postgres_db_manager.add_connection("message_metadata_db", config._msg_metadata_db_connection_string);

	_user_metadata_repo = std::make_unique<UserMetadataRepository>(_postgres_db_manager, "user_metadata_db");
	_msg_metadata_repo = std::make_unique<MessageMetadataRepository>(_postgres_db_manager, "message_metadata_db");
	_msg_text_repo = std::make_unique<MessageTextRepository>(config._msg_text_db_connection_string);
	_file_server_client = std::make_unique<FileServerClient>(config._file_server_host, config._file_server_port);
}

int RepositoryManager::create_user(const User& user) {
	return _user_metadata_repo->create(user);
}

bool RepositoryManager::authorize_user(int user_id, const std::string& nickname, const std::string& password) {
	return _user_metadata_repo->authorize(user_id, nickname, password);
}

int RepositoryManager::get_user_id(const std::string& nickname) {
	return _user_metadata_repo->get_id(nickname);
}

int RepositoryManager::create_message(const Message& message) {
	int msg_metadata_id = _msg_metadata_repo->create(message.get_metadata());
	// int msg_text_id = _msg_text_repo->create(message.get_text());
	return msg_metadata_id;
}

bool RepositoryManager::upload_file_chunk(const std::string& filename, const std::string& chunk_data) {
	return _file_server_client->upload_chunk(filename, chunk_data);
}

std::vector<std::string> RepositoryManager::download_file_chunks(const std::string& filename) {
	return _file_server_client->download_file_chunks(filename);
}