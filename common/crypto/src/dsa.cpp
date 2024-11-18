#include "dsa.hpp"

namespace common {
namespace crypto {

DSA::DSA(const cpp_int& prime_modulus, const cpp_int& prime_divisor, const cpp_int& generator)
	: _p(prime_modulus), _q(prime_divisor), _g(generator) {

	if (!validate_dsa_parameters(_p, _q, _g)) {
		FATAL_MSG("[DSA::DSA] Invalid parameters!");
		throw std::invalid_argument("Invalid DSA parameters");
	}

	_keys._private_key = generate_random(1, _q - 1);
	_keys._public_key = modular_pow(_g, _keys._private_key, _p);

	DEBUG_MSG("[DSA::DSA] Initialized successfully. \npublic key: " + cpp_int_to_hex(_keys._public_key) +
	          "\nprivate key: " + cpp_int_to_hex(_keys._private_key));
}

DSASignature DSA::sign(const cpp_int& message_hash) {
	DSASignature signature;
	cpp_int k;

	while (signature._signature == 0) {
		k = generate_random(1, _q - 1);
		signature._r = modular_pow(_g, k, _p) % _q;

		if (signature._r == 0) {
			continue;
		}

		cpp_int k_inv = modular_pow(k, _q - 2, _q);
		signature._signature = (k_inv * (message_hash + _keys._private_key * signature._r)) % _q;
	}

	DEBUG_MSG("[DSA::sign] Signature generated successfully. \nr: " + cpp_int_to_hex(signature._r) +
	          "\ns: " + cpp_int_to_hex(signature._signature));

	return signature;
}

bool DSA::verify(const cpp_int& message_hash, const DSASignature& signature, const cpp_int& signer_public_key) {
	if (signature._r <= 0 || signature._r >= _q || signature._signature <= 0 || signature._signature >= _q) {
		DEBUG_MSG("[DSA::verify] Invalid signature values");
		return false;
	}

	cpp_int w = modular_pow(signature._signature, _q - 2, _q);
	cpp_int u1 = (message_hash * w) % _q;
	cpp_int u2 = (signature._r * w) % _q;
	cpp_int v = (modular_pow(_g, u1, _p) * modular_pow(signer_public_key, u2, _p)) % _p % _q;

	bool is_valid = (v == signature._r);

	DEBUG_MSG("[DSA::verify] Signature verification " + std::string(is_valid ? "successful" : "failed"));

	return is_valid;
}

cpp_int DSA::get_public_key() const {
	return _keys._public_key;
}

const KeyPair& DSA::get_keys() const {
	return _keys;
}

} // namespace common
} // namespace crypto