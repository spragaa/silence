#include "message_text_repository.hpp"

#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

class MessageTextRepositoryTestEnvironment : public ::testing::Environment {
	void SetUp() override {
		DEBUG_MSG("Setting up MessageTextRepositoryTestEnvironment...");
		DEBUG_MSG("Executing " + std::string(SOURCE_DIR) + "/tests/test_server/scripts/setup_test_message_text.sh");
		// creates warning
		// this one asks for pass, which is annoying
		// mb move test db somewhere?
		system(("sudo " + std::string(SOURCE_DIR) + "/tests/test_server/scripts/setup_test_message_text.sh").c_str());
	}

	void TearDown() override {
		INFO_MSG("Cleaning MessageTextRepositoryTestEnvironment...");
		DEBUG_MSG("Executing " + std::string(SOURCE_DIR) + "/tests/test_server/scripts/drop_test_message_text.sh");
		// creates warning
		system(("sudo " + std::string(SOURCE_DIR) + "/tests/test_server/scripts/drop_test_message_text.sh").c_str());
	}
};