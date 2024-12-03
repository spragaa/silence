// #include <gtest/gtest.h>
// #include "message.hpp"
// #include <chrono>
// #include <nlohmann/json.hpp>

// class MessageTests : public ::testing::Test {
// protected:
//     int sender_id;
//     int receiver_id;
//     std::string text;
//     common::Message msg;

//     void SetUp() override {
//         sender_id = 1;
//         receiver_id = 2;
//         text = "Zig is future?";
//         msg = common::Message(sender_id, receiver_id, text);
//     }

//     common::Timestamp create_timestamp() {
//         return std::chrono::system_clock::now();
//     }
// };

// TEST_F(MessageTests, constructor_test) {
//     EXPECT_EQ(msg.get_sender_id(), sender_id);
//     EXPECT_EQ(msg.get_receiver_id(), receiver_id);
//     EXPECT_EQ(msg.get_text(), text);
//     EXPECT_FALSE(msg.is_deleted());
// }

// TEST_F(MessageTests, setters_and_getters_test) {
//     msg.set_text("Zig is the guy");
//     EXPECT_EQ(msg.get_text(), "Zig is the guy");

//     msg.set_sender_id(3);
//     EXPECT_EQ(msg.get_sender_id(), 3);

//     common::Timestamp ts = create_timestamp();
//     msg.set_created_timestamp(ts);
//     EXPECT_EQ(msg.get_created_timestamp(), ts);

//     msg.set_deleted(true);
//     EXPECT_TRUE(msg.is_deleted());
// }

// TEST_F(MessageTests, timestamps_test) {
//     common::Timestamp created_ts = create_timestamp();
//     msg.set_created_timestamp(created_ts);
//     EXPECT_EQ(msg.get_created_timestamp(), created_ts);

//     common::Timestamp edited_ts = create_timestamp();
//     msg.set_last_edited_timestamp(edited_ts);
//     EXPECT_EQ(msg.get_last_edited_timestamp().value(), edited_ts);

//     Timestamp deleted_ts = create_timestamp();
//     msg.set_deleted_timestamp(deleted_ts);
//     EXPECT_EQ(msg.get_deleted_timestamp().value(), deleted_ts);
// }

// TEST_F(MessageTests, JsonSerializationTest) {
//     msg.set_deleted(true);

//     nlohmann::json j = msg.to_json();
//     EXPECT_EQ(j["sender_id"], sender_id);
//     EXPECT_EQ(j["receiver_id"], receiver_id);
//     EXPECT_EQ(j["text"], text);
//     EXPECT_TRUE(j["deleted"]);

//     common::Message deserialized_msg = common::Message::from_json(j);
//     EXPECT_EQ(deserialized_msg.get_sender_id(), sender_id);
//     EXPECT_EQ(deserialized_msg.get_receiver_id(), receiver_id);
//     EXPECT_EQ(deserialized_msg.get_text(), text);
//     EXPECT_TRUE(deserialized_msg.is_deleted());
// }