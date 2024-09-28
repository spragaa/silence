#include "user_metadata_repository.hpp"
#include <gtest/gtest.h>

class UserMetadataRepositoryTests : public ::testing::Test {
protected:
    std::unique_ptr<UserMetadataRepository> repo;
    DBManager db_manager;

    void SetUp() override {
        db_manager.add_connection("test_user_metadata", "host=localhost port=5432 dbname=test_user_metadata user=postgres password=pass");
        repo = std::make_unique<UserMetadataRepository>(db_manager, "test_user_metadata");
    }

};

TEST_F(UserMetadataRepositoryTests, read_user) {
    
}
