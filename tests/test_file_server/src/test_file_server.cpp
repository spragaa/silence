#include "file_server.hpp"

#include <gtest/gtest.h>

// CMake Error at /home/logi/.local/lib/python3.10/site-packages/cmake/data/share/cmake-3.29/Modules/GoogleTestAddTests.cmake:112 (message):
//   Error running test executable.

//     Path: '/home/logi/myself/programming/cpp/chat_application/build/tests/test_file_server/test_file_server'
//     Working directory: '/home/logi/myself/programming/cpp/chat_application/build/tests/test_file_server'
//     Result: Subprocess aborted
//     Output:
      

// Call Stack (most recent call first):
//   /home/logi/.local/lib/python3.10/site-packages/cmake/data/share/cmake-3.29/Modules/GoogleTestAddTests.cmake:226 (gtest_discover_tests_impl)

class FileServerTests : public ::testing::Test {
protected:
    void SetUp() override {
        _test_dir = std::filesystem::path(std::string(SOURCE_DIR) + "/tests/test_file_server/test_media_file_system");
        std::filesystem::create_directories(_test_dir);
        
        _server = std::make_unique<file_server::FileServer>(
            54444, 
            4, 
            _test_dir.string(), 
            1024*1024*1024
        );
        
        _server->init();
        _server_thread = std::thread([this]() {
           _server->start(); 
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
    
    void TearDown() override {
        std::filesystem::remove_all(_test_dir);
        
        if (_server_thread.joinable()) {
            _server_thread.join();
        }
    }
    
    std::filesystem::path _test_dir;
    std::unique_ptr<file_server::FileServer> _server;
    std::thread _server_thread;
};

TEST_F(FileServerTests, is_valid_filename) {
    EXPECT_TRUE(_server->is_valid_filename("abcDEF123456789"));
    EXPECT_TRUE(_server->is_valid_filename("ABCDEF1234567890"));
    EXPECT_FALSE(_server->is_valid_filename("abc"));
    EXPECT_FALSE(_server->is_valid_filename("abc!@#$%^&*()"));
    EXPECT_FALSE(_server->is_valid_filename("")); 
}