#include "message_metadata_repository.hpp"

#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

class MessageMetadataRepositoryTestEnvironment : public ::testing::Environment {
	void SetUp() override {
		DEBUG_MSG("Setting up MessageMetadataRepositoryTestEnvironment...");
		DEBUG_MSG("Executing " + std::string(SOURCE_DIR) + "/tests/test_server/scripts/setup_test_message_metadata.sh");
		system((std::string(SOURCE_DIR) + "/tests/test_server/scripts/setup_test_message_metadata.sh --user=spraga --user_password=spraga").c_str());
	}

	void TearDown() override {
		INFO_MSG("Cleaning MessageMetadataRepositoryTestEnvironment...");
		DEBUG_MSG("Executing " + std::string(SOURCE_DIR) + "/tests/test_server/scripts/drop_test_message_metadata.sh");
		system((std::string(SOURCE_DIR) + "/tests/test_server/scripts/drop_test_message_metadata.sh").c_str());
	}
};