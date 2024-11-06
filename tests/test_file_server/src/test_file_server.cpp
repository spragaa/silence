#include "file_server.hpp"

#include <gtest/gtest.h>

class FileServerTests : public ::testing::Test {
protected:
    void SetUp() override {
        _server = std::make_unique<file_server::FileServer>(
            54444, 
            4
        );
        
        _server_thread = std::thread([this]() {
           _server->start(); 
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
    
    void TearDown() override {
        if (_server) {
            _server->stop();
        }
        
        if (_server_thread.joinable()) {
            _server_thread.join();
        }
        
        _server.reset();
    }
    
    std::unique_ptr<file_server::FileServer> _server;
    std::thread _server_thread;
};

TEST_F(FileServerTests, test1) {
    EXPECT_EQ(1, 1);
}

TEST_F(FileServerTests, test2) {
    EXPECT_EQ(2, 2);
}