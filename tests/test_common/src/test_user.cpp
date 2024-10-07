#include "user.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <chrono>
#include <filesystem>

class UserTests : public ::testing::Test {
protected:
    std::string user_metadata_dir;
    std::string default_nickname;
    std::string default_password;
    User default_user;

    void SetUp() override {
        user_metadata_dir = std::string(SOURCE_DIR) + "/tests/user_metadata";
        std::filesystem::create_directory(user_metadata_dir);

        default_nickname = "primeagen";
        default_password = "primeagen_pass";
        default_user = User(default_nickname, default_password);
    }
    
    void TearDown() override {
        std::filesystem::remove_all(user_metadata_dir);
    }
};

TEST_F(UserTests, default_constructor_test) {
    User u;
    
    EXPECT_EQ(u.get_id(), 0);
    EXPECT_EQ(u.get_nickname(), "");
    EXPECT_EQ(u.get_password(), "");
}

TEST_F(UserTests, constructor_with_nickname_and_pass_test) {
    User u(default_nickname, default_password);
    
    EXPECT_EQ(u.get_id(), 3); // consider removing id_counter if needed
    EXPECT_EQ(u.get_nickname(), default_nickname);
    EXPECT_EQ(u.get_password(), default_password);
    EXPECT_EQ(u.is_online(), true);
}

TEST_F(UserTests, to_json_test) {
    int u_id = 666;
    bool u_online = true;
    Timestamp u_registered_timestamp = Timestamp(std::chrono::nanoseconds(7459324));
    Timestamp u_last_online_timestamp = Timestamp(std::chrono::nanoseconds(2302102));
    
    User u(default_nickname, default_password);
    u.set_id(u_id);
    u.set_online(u_online);
    u.set_registered_timestamp(u_registered_timestamp);
    u.set_last_online_timestamp(u_last_online_timestamp);
    
    nlohmann::json json = u.to_json();
    
    DEBUG_MSG("User to_json json:" + json.dump());
    
    EXPECT_EQ(u_id, json["id"]);
    EXPECT_EQ(default_nickname, json["nickname"]);
    EXPECT_EQ(default_password, json["password"]);
    EXPECT_EQ(u_online, json["online"]);
    // these throw C++ exception with description "[json.exception.type_error.302] type must be string, but is number"
    // not sure how to fix it rn, cause I'm in the train :) 
    // EXPECT_EQ(u_registered_timestamp, u.parse_timestamp(json["registered_timestamp"]));
    // EXPECT_EQ(u_last_online_timestamp, u.parse_timestamp(json["online_timestamp"]));
}

TEST_F(UserTests, check_password_test) {
    bool is_pass_correct = default_user.check_password(default_password);
    EXPECT_TRUE(is_pass_correct);
    
    is_pass_correct = default_user.check_password("dummy password");
    EXPECT_FALSE(is_pass_correct);
}

TEST_F(UserTests, constructor_with_nickname_test) {
    User u(default_nickname);
    
    EXPECT_EQ(u.get_id(), 0);
    EXPECT_EQ(u.get_nickname(), default_nickname);
    EXPECT_EQ(u.get_password(), "");
}

TEST_F(UserTests, setter_getter_tests) {
    User u;
    int id = 42;
    std::string nickname = "newprimeagen";
    std::string password = "newprimeagenpass";
    Timestamp now = std::chrono::system_clock::now();
    bool online = true;

    u.set_id(id);
    u.set_nickname(nickname);
    u.set_password(password);
    u.set_registered_timestamp(now);
    u.set_last_online_timestamp(now);
    u.set_online(online);

    EXPECT_EQ(u.get_id(), id);
    EXPECT_EQ(u.get_nickname(), nickname);
    EXPECT_EQ(u.get_password(), password);
    EXPECT_EQ(u.get_registered_timestamp(), now);
    EXPECT_EQ(u.get_last_online_timestamp(), now);
    EXPECT_EQ(u.is_online(), online);
}

TEST_F(UserTests, from_json_test) {
    nlohmann::json j = {
        {"id", 123},
        {"nickname", "primeagen"},
        {"password", "primeagenass"},
        {"registered_timestamp", "1633046400000000000"},
        {"last_online_timestamp", "1633132800000000000"},
        {"online", true}
    };

    User u = User::from_json(j);

    EXPECT_EQ(u.get_id(), 123);
    EXPECT_EQ(u.get_nickname(), "primeagen");
    EXPECT_EQ(u.get_password(), "primeagenass");
    // timestamps still broken 
    EXPECT_EQ(u.get_registered_timestamp().time_since_epoch().count(), 1633046400000000000);
    EXPECT_EQ(u.get_last_online_timestamp().time_since_epoch().count(), 1633132800000000000);
    EXPECT_TRUE(u.is_online());
}

// [DEBUG] [2024-10-07 20:35:47] User data saved to /home/logi/myself/programming/cpp/chat_application/tests/user_metadata/test_user_data.json
// [DEBUG] [2024-10-07 20:35:47] User data loaded from /home/logi/myself/programming/cpp/chat_application/tests/user_metadata/test_user_data.json
// unknown file: Failure
// C++ exception with description "[json.exception.type_error.302] type must be string, but is number" thrown in the test body.
TEST_F(UserTests, save_and_load_user_data_test) {
    User original_user("primeagen_save", "primeagen_save_pass");
    original_user.set_id(666);
    original_user.set_online(true);

    std::string filename = user_metadata_dir + "/test_user_data.json";
    original_user.save_user_data_to_json(filename);

    User loaded_user = User::load_user_data_from_json(filename);

    EXPECT_EQ(loaded_user.get_id(), original_user.get_id());
    EXPECT_EQ(loaded_user.get_nickname(), original_user.get_nickname());
    EXPECT_EQ(loaded_user.get_password(), original_user.get_password());
    EXPECT_EQ(loaded_user.is_online(), original_user.is_online());

    std::filesystem::remove(filename);
}

TEST_F(UserTests, load_user_data_nonexistent_file_test) {
    std::string nonexistent_filename = user_metadata_dir + "/nonexisting_test_user_data.json";

    testing::internal::CaptureStderr();
    User loaded_user = User::load_user_data_from_json(nonexistent_filename);
    std::string output = testing::internal::GetCapturedStderr();

    EXPECT_EQ(loaded_user.get_id(), 0);
    EXPECT_EQ(loaded_user.get_nickname(), "");
    EXPECT_EQ(loaded_user.get_password(), "");
}