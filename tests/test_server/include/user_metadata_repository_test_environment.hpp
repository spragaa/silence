#include "user_metadata_repository.hpp"
#include "message_metadata_repository.hpp"

#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

class UserMetadataRepositoryTestEnvironment : public ::testing::Environment {
    void SetUp() override {
        DEBUG_MSG("Setting up UserMetadataRepositoryTestEnvironment...");
        DEBUG_MSG("Executing " + std::string(SOURCE_DIR) + "/tests/test_server/scripts/setup_test_user_metadata.sh");
        // --user=spraga --user_password=spraga -- should read from params
        system((std::string(SOURCE_DIR) + "/tests/test_server/scripts/setup_test_user_metadata.sh --user=spraga --user_password=spraga").c_str());
    }
    
    void TearDown() override {
        DEBUG_MSG("Cleaning UserMetadataRepositoryTestEnvironment...");
        DEBUG_MSG("Executing " + std::string(SOURCE_DIR) + "/tests/test_server/scripts/drop_test_user_metadata.sh");
        system((std::string(SOURCE_DIR) + "/tests/test_server/scripts/drop_test_user_metadata.sh").c_str());
    }
};