#include "message_metadata_repository.hpp"

#include <gtest/gtest.h>

class MessageMetadataRepositoryTests : public ::testing::Test {
protected:
	std::unique_ptr<MessageMetadataRepository> _repo;
	PostgresDBManager _postgres_db_manager;

	void SetUp() override {
		_postgres_db_manager.add_connection("test_message_metadata", "host=localhost port=5432 dbname=test_message_metadata user=postgres password=pass");
		_repo = std::make_unique<MessageMetadataRepository>(_postgres_db_manager, "test_message_metadata");
	}
};

TEST_F(MessageMetadataRepositoryTests, create_message) {
	MessageMetadata message(100, 1, 2);
	int message_id = _repo->create(message);

	ASSERT_EQ(message_id, 100);

	std::optional<MessageMetadata> retrieved_message = _repo->read(message_id);
	ASSERT_TRUE(retrieved_message.has_value());

	EXPECT_EQ(retrieved_message->get_sender_id(), 1);
	EXPECT_EQ(retrieved_message->get_receiver_id(), 2);
}

TEST_F(MessageMetadataRepositoryTests, read_non_existing_message) {
	std::optional<MessageMetadata> message = _repo->read(9999);
	ASSERT_FALSE(message.has_value());
}

// since I remove text from the MessageMetadata class, here we should update the metadata
// commenting as not relevant for now
// TEST_F(MessageMetadataRepositoryTests, update_message) {
// 	MessageMetadata message(1, 2);
// 	int message_id = _repo->create(message);

// 	ASSERT_GT(message_id, 0);

// 	message = MessageMetadata(1, 2);
// 	message.set_id(message_id);
// 	bool is_updated = _repo->update(message);

// 	ASSERT_TRUE(is_updated);

// 	std::optional<MessageMetadata> updated_message = _repo->read(message_id);
// 	ASSERT_TRUE(updated_message.has_value());
// }

TEST_F(MessageMetadataRepositoryTests, remove_message) {
	MessageMetadata message(666, 1, 2);
	int message_id = _repo->create(message);

	ASSERT_EQ(message_id, 666);

	bool is_removed = _repo->remove(message_id);
	ASSERT_TRUE(is_removed);

	std::optional<MessageMetadata> deleted_message = _repo->read(message_id);
	ASSERT_TRUE(deleted_message.has_value());
	EXPECT_EQ(deleted_message->get_sender_id(), 1);
	EXPECT_EQ(deleted_message->get_receiver_id(), 2);
	EXPECT_EQ(deleted_message->is_deleted(), true);
}