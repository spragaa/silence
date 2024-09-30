#include "message_metadata_repository.hpp"

#include <gtest/gtest.h>
#include <iostream>

class MessageMetadataRepositoryTests : public ::testing::Test {
protected:
    std::unique_ptr<MessageMetadataRepository> repo;
    DBManager db_manager;

    void SetUp() override {
        db_manager.add_connection("test_message_metadata", "host=localhost port=5432 dbname=test_message_metadata user=postgres password=pass");
        repo = std::make_unique<MessageMetadataRepository>(db_manager, "test_message_metadata");
    }
};

TEST_F(MessageMetadataRepositoryTests, create_message) {
    Message message(1, 2, "Hello, World!");
    int message_id = repo->create(message);

    ASSERT_GT(message_id, 0);

    std::optional<Message> retrieved_message = repo->read(message_id);
    ASSERT_TRUE(retrieved_message.has_value());

    EXPECT_EQ(retrieved_message->get_sender_id(), 1);
    EXPECT_EQ(retrieved_message->get_receiver_id(), 2);
    EXPECT_EQ(retrieved_message->get_text(), "Hello, World!");
}

TEST_F(MessageMetadataRepositoryTests, read_non_existing_message) {
    std::optional<Message> message = repo->read(9999);
    ASSERT_FALSE(message.has_value());
}

TEST_F(MessageMetadataRepositoryTests, update_message) {
    Message message(1, 2, "Initial Text");
    int message_id = repo->create(message);

    ASSERT_GT(message_id, 0);

    message = Message(1, 2, "Updated Text");
    message.set_id(message_id);
    bool is_updated = repo->update(message);

    ASSERT_TRUE(is_updated);

    std::optional<Message> updated_message = repo->read(message_id);
    ASSERT_TRUE(updated_message.has_value());
    EXPECT_EQ(updated_message->get_text(), "Updated Text");
}

TEST_F(MessageMetadataRepositoryTests, remove_message) {
    Message message(1, 2, "To be deleted");
    int message_id = repo->create(message);

    ASSERT_GT(message_id, 0);

    bool is_removed = repo->remove(message_id);
    ASSERT_TRUE(is_removed);

    std::optional<Message> deleted_message = repo->read(message_id);
    ASSERT_FALSE(deleted_message.has_value());
}