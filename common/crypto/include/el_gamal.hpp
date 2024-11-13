#pragma once

#include "crypto_utils.hpp"

#include <boost/multiprecision/cpp_int.hpp>
#include <random>
#include <array>
#include <vector>

namespace  common {
namespace crypto {

class ElGamal {
public:
	using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_off>;

	ElGamal(const cpp_int& prime_modulus, const cpp_int& generator);

	const KeyPair& get_keys() const;
	EncryptedMessage encrypt(const cpp_int& message, const cpp_int& recipient_public_key);
	cpp_int decrypt(const EncryptedMessage& encrypted_message);

protected:
	cpp_int generate_random(const cpp_int& min, const cpp_int& max);
	cpp_int modular_pow(const cpp_int& base, const cpp_int& exponent, const cpp_int& modulus);

private:
	cpp_int p;
	cpp_int g;
	KeyPair keys;
};

std::string format_key_info(const std::string& label, const common::crypto::KeyPair& keys);
std::string format_encrypted_message(const std::string& label, const common::crypto::EncryptedMessage& msg);

} // namespace common
} // namespace crypto