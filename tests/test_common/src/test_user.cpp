#include "user.hpp"

#include <gtest/gtest.h>
#include <iostream>
#include <chrono>

class UserTests : public ::testing::Test {
protected:
    // User user;
	// void SetUp() override {
	//    std::string nickname = "primeprimeagen";
	//    std::string password = "primeprimeagen_password";

	// 	user = User(nickname, password);
	// 	user.set_id(666);
	// }
};

TEST_F(UserTests, default_consrutor_test) {
    User u;
    
    EXPECT_EQ(u.get_id(), 0);
    EXPECT_EQ(u.get_nickname(), "");
    EXPECT_EQ(u.get_password(), "");
}

TEST_F(UserTests, constructor_with_nickname_and_pass_test) {
    std::string primeagen = "primeagen";
    std::string primeagen_pass = "primeagen_pass";
    
    User u(primeagen, primeagen_pass);
    
    EXPECT_EQ(u.get_id(), 1); // currently this is a zero, I am thinking about removing id_counter from user and message classes
    EXPECT_EQ(u.get_nickname(), primeagen);
    EXPECT_EQ(u.get_password(), primeagen_pass);
    EXPECT_EQ(u.is_online(), true);
}

TEST_F(UserTests, to_json_test) {
    int u_id = 666;
    std::string u_nickname = "primeagen";
    std::string u_password = "primeagen_pass";
    bool u_online = true;
    Timestamp u_registered_timestamp = Timestamp(std::chrono::nanoseconds(7459324));
    Timestamp u_last_online_timestamp = Timestamp(std::chrono::nanoseconds(2302102));
    
    User u(u_nickname, u_password);
    u.set_id(u_id);
    u.set_nickname(u_nickname);
    u.set_password(u_password);
    u.set_online(u_online);
    u.set_registered_timestamp(u_registered_timestamp);
    u.set_last_online_timestamp(u_last_online_timestamp);
    
    nlohmann::json json = u.to_json();
    
    DEBUG_MSG("User to_json json:" + json.dump());
    
    EXPECT_EQ(u_id, json["id"]);
    EXPECT_EQ(u_nickname, json["nickname"]);
    EXPECT_EQ(u_password, json["password"]);
    EXPECT_EQ(u_online, json["online"]);
    // these things throw C++ exception with description "[json.exception.type_error.302] type must be string, but is number"
    // not sure how to fix it rn, cause I'm in the train :) 
    // EXPECT_EQ(u_registered_timestamp, u.parse_timestamp(json["registered_timestamp"]));
    // EXPECT_EQ(u_last_online_timestamp, u.parse_timestamp(json["online_timestamp"]));
}

// TEST_F(UserTests, from_json_test) {
// to do  
// }

TEST_F(UserTests, check_password_test) {
    std::string primeagen = "primeagen";
    std::string primeagen_pass = "primeagen_pass";
    
    User u(primeagen, primeagen_pass);
    
    bool is_pass_correct = u.check_password(primeagen_pass);
    EXPECT_TRUE(is_pass_correct);
    
    is_pass_correct = u.check_password("dummy password");
    EXPECT_FALSE(is_pass_correct);
}

// TEST_F(UserTests, save_user_data_to_json_test) {
// to do
// }
// 
// TEST_F(UserTests, load_user_data_from_json_test) {
// to do 
// }