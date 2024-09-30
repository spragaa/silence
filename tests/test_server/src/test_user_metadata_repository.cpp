#include "user_metadata_repository.hpp"

#include <gtest/gtest.h>
#include <iostream>

class UserMetadataRepositoryTests : public ::testing::Test {
protected:
    std::unique_ptr<UserMetadataRepository> repo;
    DBManager db_manager;

    void SetUp() override {
        db_manager.add_connection("test_user_metadata", "host=localhost port=5432 dbname=test_user_metadata user=postgres password=pass");
        repo = std::make_unique<UserMetadataRepository>(db_manager, "test_user_metadata");
    }
};

TEST_F(UserMetadataRepositoryTests, read_user_with_id_1) {
    int user_id = 1;
    std::optional<User> user = repo->read(user_id);

    ASSERT_TRUE(user.has_value());

    auto n_user = *user;
    nlohmann::json user_json = n_user.to_json();

    DEBUG_MSG("Response: " + user_json.dump(2));

    EXPECT_EQ(user_json["id"], 1);
    EXPECT_EQ(user_json["nickname"], "linus_torvalds");
    EXPECT_EQ(user_json["password"], "linus_torvalds_pass");
    EXPECT_TRUE(user_json["online"].get<bool>());
    EXPECT_EQ(user_json["registered_timestamp"], 1727521200000000000);
    EXPECT_EQ(user_json["last_online_timestamp"], 1727521200000000000);
}

TEST_F(UserMetadataRepositoryTests, create_user) {
    int user_id = 5;
    std::string nickname = "the_primeagen";
    std::string pass = "the_primeagen_pass";
    std::string timestamp_str = "2024-09-28 15:30:00";
    bool online = true;

    std::tm tm = {};
    std::istringstream ss(timestamp_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    User user_to_create(nickname, pass);
    user_to_create.set_id(user_id);
    user_to_create.set_registered_timestamp(timestamp);
    user_to_create.set_last_online_timestamp(timestamp);
    user_to_create.set_online(online);

    bool is_created = repo->create(user_to_create);

    ASSERT_TRUE(is_created);

    std::optional<User> user = repo->read(user_id);
    ASSERT_TRUE(user.has_value());

    auto n_user = *user;
    nlohmann::json user_json = n_user.to_json();

    DEBUG_MSG("Response: " + user_json.dump(2));

    EXPECT_EQ(user_json["id"], user_id);
    EXPECT_EQ(user_json["nickname"], nickname);
    EXPECT_EQ(user_json["password"], pass);
    EXPECT_TRUE(user_json["online"].get<bool>());
    EXPECT_EQ(user_json["registered_timestamp"], 1727523000000000000);
    EXPECT_EQ(user_json["last_online_timestamp"], 1727523000000000000);
}

TEST_F(UserMetadataRepositoryTests, remove_existing_user) {
    int user_id = 1;
    bool is_removed = repo->remove(1);

    ASSERT_TRUE(is_removed);

    std::optional<User> user = repo->read(user_id);
    ASSERT_FALSE(user.has_value());
}

TEST_F(UserMetadataRepositoryTests, remove_non_existing_user) {
    int user_id = 6;
    bool is_removed = repo->remove(1);

    ASSERT_FALSE(is_removed);

    std::optional<User> user = repo->read(user_id);
    ASSERT_FALSE(user.has_value());
}

TEST_F(UserMetadataRepositoryTests, update_existing_user) {
    int user_id = 5;
    std::string nickname = "the_primeagen_goat";
    std::string pass = "the_primeagen_goat_pass";
    std::string timestamp_str = "2024-09-28 15:30:00";
    bool online = true;

    std::tm tm = {};
    std::istringstream ss(timestamp_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    User user_to_update(nickname, pass);
    user_to_update.set_id(user_id);
    user_to_update.set_registered_timestamp(timestamp);
    user_to_update.set_last_online_timestamp(timestamp);
    user_to_update.set_online(online);

    bool is_updated = repo->update(user_to_update);

    ASSERT_TRUE(is_updated);

    std::optional<User> user = repo->read(user_id);
    ASSERT_TRUE(user.has_value());

    auto n_user = *user;
    nlohmann::json user_json = n_user.to_json();

    DEBUG_MSG("Response: " + user_json.dump(2));

    EXPECT_EQ(user_json["id"], user_id);
    EXPECT_EQ(user_json["nickname"], nickname);
    EXPECT_EQ(user_json["password"], pass);
    EXPECT_TRUE(user_json["online"].get<bool>());
    EXPECT_EQ(user_json["registered_timestamp"], 1727523000000000000);
    EXPECT_EQ(user_json["last_online_timestamp"], 1727523000000000000);
}

TEST_F(UserMetadataRepositoryTests, update_non_existing_user) {
    int user_id = 6;
    std::string nickname = "primeagen_the__goat";
    std::string pass = "primeagen_the_goat_pass";
    std::string timestamp_str = "2024-09-28 15:30:00";
    bool online = true;

    std::tm tm = {};
    std::istringstream ss(timestamp_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto timestamp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

    User user_to_update(nickname, pass);
    user_to_update.set_id(user_id);
    user_to_update.set_registered_timestamp(timestamp);
    user_to_update.set_last_online_timestamp(timestamp);
    user_to_update.set_online(online);

    bool is_updated = repo->update(user_to_update);

    ASSERT_FALSE(is_updated);

    std::optional<User> user = repo->read(user_id);
    ASSERT_FALSE(user.has_value());
}
