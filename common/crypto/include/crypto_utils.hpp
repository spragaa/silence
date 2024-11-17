#pragma once

#include "debug.hpp"

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

struct DSASignature {
	cpp_int r;
	cpp_int signature;
};

cpp_int hex_to_cpp_int(const std::string& hex);
std::string cpp_int_to_hex(const cpp_int& num);
cpp_int find_valid_generator(const cpp_int& p);
bool is_prime(const cpp_int& n, int iterations = 50);
bool validate_el_gamal_parameters(const cpp_int& p, const cpp_int& g);
cpp_int find_generator(const cpp_int& p);
cpp_int generate_random(const cpp_int& min, const cpp_int& max);
bool miller_rabin_test(const cpp_int& n, const cpp_int& a);
cpp_int modular_pow(const cpp_int& base, const cpp_int& exponent, const cpp_int& modulus);
bool validate_dsa_parameters(const cpp_int& p, const cpp_int& q, const cpp_int& g);

template<size_t n>
std::string bytes_array_to_string(const std::array<uint8_t, n>& arr) {
	std::stringstream ss;
	ss << std::hex << std::setfill('0');

	for (const auto& byte : arr) {
		ss << std::setw(2) << static_cast<int>(byte);
	}

	return ss.str();
}

template<size_t n>
std::array<uint8_t, n / 8> generate_aes_key() {
	static_assert(n == 128 || n == 192 || n == 256, "Key size must be 128, 192, or 256 bits");
	DEBUG_MSG("[aes256::generate_key] Key size must be 128, 192, or 256 bits");

	std::array<uint8_t, n / 8> key;
	std::random_device rd;

	for(size_t i = 0; i < key.size(); ++i) {
		key[i] = static_cast<uint8_t>(rd());
	}

	DEBUG_MSG("[aes256::generate_key] Generated key: " + bytes_array_to_string(key));

	return key;
}

} // namespace common
} // namespace crypto