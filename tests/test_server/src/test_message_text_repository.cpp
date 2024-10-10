#include "message_text_repository.hpp"

#include <gtest/gtest.h>

class MessageTextRepositoryTests : public ::testing::Test {
protected:
	std::unique_ptr<MessageTextRepository> repo;

	void SetUp() override {
		repo = std::make_unique<MessageTextRepository>("redis://:spraga@127.0.0.1:6380");
	}
};

TEST_F(MessageTextRepositoryTests, testsatsat) {
    EXPECT_EQ(1, 1);
}