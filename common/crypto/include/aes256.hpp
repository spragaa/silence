#pragma once

#include "debug.hpp"

#include <string>
#include <array>
#include <cstddef>

std::string aes256_encrypt(const std::string& input, const std::array<uint8_t, 32>& key);
std::string aes256_decrypt(const std::string& input, const std::array<uint8_t, 32>& key);