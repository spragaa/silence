#include "user_metadata_repository_test_environment.hpp"
#include "message_metadata_repository_test_environment.hpp"
#include "message_text_repository_test_environment.hpp"

#include <gtest/gtest.h>

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);

	::testing::AddGlobalTestEnvironment(new MessageMetadataRepositoryTestEnvironment);
	::testing::AddGlobalTestEnvironment(new UserMetadataRepositoryTestEnvironment);

	// cannot setup redis db for testing along side with production db :(
	// ::testing::AddGlobalTestEnvironment(new MessageTextRepositoryTestEnvironment);

	return RUN_ALL_TESTS();
}