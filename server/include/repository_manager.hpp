#pragma once

#include "user_metadata_repository.hpp"
#include "message_metadata_repository.hpp"
#include "message_text_repository.hpp"
#include "file_server_client.hpp"
#include "postgres_db_manager.hpp"
#include "server_config.hpp"
#include "user.hpp"
#include "message.hpp"

#include <memory>
#include <string>
#include <vector>

class RepositoryManager {
public:
    explicit RepositoryManager(const ServerConfig& config);
    ~RepositoryManager() = default;

    int create_user(const User& user);
    bool authorize_user(int user_id, const std::string& nickname, const std::string& password);
    int get_user_id(const std::string& nickname);

    int create_message(const Message& message);
    Message get_message(int message_id);

    bool upload_file_chunk(const std::string& filename, const std::string& chunk_data);
    std::vector<std::string> download_file_chunks(const std::string& filename);

private:
    PostgresDBManager _postgres_db_manager;
    std::unique_ptr<UserMetadataRepository> _user_metadata_repo;
    std::unique_ptr<MessageMetadataRepository> _msg_metadata_repo;
    std::unique_ptr<MessageTextRepository> _msg_text_repo;
    std::unique_ptr<FileServerClient> _file_server_client;
};