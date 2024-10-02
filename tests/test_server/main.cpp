#include "user_metadata_repository_test_environment.hpp"
#include "message_metadata_repository_test_environment.hpp"

#include <gtest/gtest.h>

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);

	::testing::AddGlobalTestEnvironment(new UserMetadataRepositoryTestEnvironment);
	::testing::AddGlobalTestEnvironment(new MessageMetadataRepositoryTestEnvironment);
	return RUN_ALL_TESTS();
}