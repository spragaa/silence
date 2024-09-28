#pragma once

#include "base_repository.hpp"
#include "message.hpp"
#include "db_manager.hpp"
#include <vector>
#include <optional>

class MessageMetadataRepository : public BaseRepository<Message> {
public:
    MessageMetadataRepository(DBManager& db_manager, const std::string& connection_name);
    virtual ~MessageMetadataRepository() override;

    int create(const Message& message) override;
    std::optional<Message> read(int id) override;
    bool update(const Message& message) override;
    bool remove(int id) override;

    std::vector<Message> getMessagesBetweenUsers(int user1_id, int user2_id);

private:
    Message constructMessage(const pqxx::row& row);
    // move inside base repo
    std::string connection_name;
};