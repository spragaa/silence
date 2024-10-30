#include "message_text_repository.hpp"

#include <gtest/gtest.h>

class MessageTextRepositoryTests : public ::testing::Test {
protected:
	std::unique_ptr<server::MessageTextRepository> _repo;

	void SetUp() override {
		_repo = std::make_unique<server::MessageTextRepository>("redis://spraga@127.0.0.1:6380");
	}
};

TEST_F(MessageTextRepositoryTests, create_test) {
	common::MessageText msg(1, "Test message");
	int result = _repo->create(msg);
	EXPECT_EQ(result, 1);

	auto retrieved = _repo->read(1);
	ASSERT_TRUE(retrieved.has_value());
	EXPECT_EQ(retrieved->get_id(), 1);
	EXPECT_EQ(retrieved->get_text(), "Test message");
}

TEST_F(MessageTextRepositoryTests, read_non_existent_messag_text) {
	auto result = _repo->read(42069);
	EXPECT_FALSE(result.has_value());
}

TEST_F(MessageTextRepositoryTests, update_existing_message) {
	common::MessageText msg(2, "Original message");
	_repo->create(msg);

	common::MessageText updated_msg(2, "Updated message");
	bool update_result = _repo->update(updated_msg);
	EXPECT_TRUE(update_result);

	auto retrieved = _repo->read(2);
	ASSERT_TRUE(retrieved.has_value());
	EXPECT_EQ(retrieved->get_text(), "Updated message");
}

TEST_F(MessageTextRepositoryTests, update_non_existing_message) {
	common::MessageText msg(999, "Non-existent message");
	bool result = _repo->update(msg);
	EXPECT_FALSE(result);
}

TEST_F(MessageTextRepositoryTests, remove_existing_message) {
	common::MessageText msg(3, "Message to be removed");
	_repo->create(msg);

	bool remove_result = _repo->remove(3);
	EXPECT_TRUE(remove_result);

	auto retrieved = _repo->read(3);
	EXPECT_FALSE(retrieved.has_value());
}

TEST_F(MessageTextRepositoryTests, remove_non_existent_message) {
	bool result = _repo->remove(999);
	EXPECT_FALSE(result);
}

TEST_F(MessageTextRepositoryTests, CreateDuplicateMessage) {
	common::MessageText msg1(4, "Original message");
	int result1 = _repo->create(msg1);
	EXPECT_EQ(result1, 4);

	common::MessageText msg2(4, "Duplicate message");
	int result2 = _repo->create(msg2);
	EXPECT_EQ(result2, 4);

	auto retrieved = _repo->read(4);
	ASSERT_TRUE(retrieved.has_value());
	EXPECT_EQ(retrieved->get_text(), "Duplicate message");
}

TEST_F(MessageTextRepositoryTests, read_after_remove) {
	common::MessageText msg(5, "Temporary message");
	_repo->create(msg);
	_repo->remove(5);

	auto result = _repo->read(5);
	EXPECT_FALSE(result.has_value());
}