#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <string>
#include <random>

namespace common {
namespace crypto {

using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_off>;

struct KeyPair {
	cpp_int private_key;
	cpp_int public_key;
};

struct EncryptedMessage {
	cpp_int c1;
	cpp_int c2;
};

cpp_int hex_to_cpp_int(const std::string& hex);
std::string cpp_int_to_hex(const cpp_int& num);
cpp_int find_valid_generator(const cpp_int& p);
bool is_prime(const cpp_int& n, int iterations = 50);
bool validate_parameters(const cpp_int& p, const cpp_int& g);
cpp_int find_generator(const cpp_int& p);
cpp_int generate_random(const cpp_int& min, const cpp_int& max);
bool miller_rabin_test(const cpp_int& n, const cpp_int& a);
cpp_int modular_pow(const cpp_int& base, const cpp_int& exponent, const cpp_int& modulus);
std::string format_key_info(const std::string& label, const common::crypto::KeyPair& keys);
std::string format_encrypted_message(const std::string& label, const common::crypto::EncryptedMessage& msg);

} // namespace common
} // namespace crypto