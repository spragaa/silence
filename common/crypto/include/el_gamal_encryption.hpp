#pragma once
#include "debug.hpp"
#include "crypto_utils.hpp"

#include <boost/multiprecision/cpp_int.hpp>
#include <array>
#include <vector>

namespace  common {
namespace crypto {

class ElGamalEncryption {
public:
	using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_off>;

	ElGamalEncryption(const cpp_int& prime_modulus, const cpp_int& generator);

	EncryptedMessage encrypt(const cpp_int& message, const cpp_int& recipient_public_key);
	cpp_int decrypt(const EncryptedMessage& encrypted_message);
	cpp_int get_public_key() const;

protected:
	const KeyPair& get_keys() const;

private:
	cpp_int _p;
	cpp_int _g;
	KeyPair _keys;
};

} // namespace common
} // namespace crypto