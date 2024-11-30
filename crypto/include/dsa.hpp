#pragma once

#include "debug.hpp"
#include "crypto_utils.hpp"

namespace crypto {

class DSA {
public:
	using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_off>;

	DSA(const cpp_int& prime_modulus, const cpp_int& prime_divisor, const cpp_int& generator);

	DSASignature sign(const cpp_int& message_hash);
	bool verify(const cpp_int& message_hash, const DSASignature& signature, const cpp_int& signer_public_key);
	cpp_int get_public_key() const;

protected:
	const KeyPair& get_keys() const;

private:
	cpp_int _p;    // prime modulus
	cpp_int _q;    // prime divisor of p-1
	cpp_int _g;    // generator
	KeyPair _keys;
};

} // namespace crypto
