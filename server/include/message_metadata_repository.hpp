#pragma once

#include "base_metadata_repository.hpp"
#include "message_metadata.hpp"
#include "postgres_db_manager.hpp"
#include <vector>
#include <optional>

namespace server {

class MessageMetadataRepository : public BaseRepository<common::MessageMetadata> {
public:
	MessageMetadataRepository(PostgresDBManager& postgres_db_manager, const std::string& connection_name);
	virtual ~MessageMetadataRepository() override;

	int create(const common::MessageMetadata& message) override;
	std::optional<common::MessageMetadata> read(int id) override;
	bool update(const common::MessageMetadata& message) override;
	bool remove(int id) override;

	std::vector<common::MessageMetadata> getMessagesBetweenUsers(int user1_id, int user2_id);

private:
	common::MessageMetadata construct_message(const pqxx::row& row);
	// move inside base repo
	std::string _connection_name;
};

}