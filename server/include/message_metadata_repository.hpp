#pragma once

#include "base_metadata_repository.hpp"
#include "message_metadata.hpp"
#include "postgres_db_manager.hpp"
#include <vector>
#include <optional>

class MessageMetadataRepository : public BaseRepository<MessageMetadata> {
public:
	MessageMetadataRepository(PostgresDBManager& postgres_db_manager, const std::string& connection_name);
	virtual ~MessageMetadataRepository() override;

	int create(const MessageMetadata& message) override;
	std::optional<MessageMetadata> read(int id) override;
	bool update(const MessageMetadata& message) override;
	bool remove(int id) override;

	std::vector<MessageMetadata> getMessagesBetweenUsers(int user1_id, int user2_id);

private:
	MessageMetadata construct_message(const pqxx::row& row);
	// move inside base repo
	std::string _connection_name;
};