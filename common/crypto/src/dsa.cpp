#include "dsa.hpp"

namespace common {
namespace crypto {

DSA::DSA(const cpp_int& prime_modulus, const cpp_int& prime_divisor, const cpp_int& generator)
	: p(prime_modulus), q(prime_divisor), g(generator) {

	if (!validate_dsa_parameters(p, q, g)) {
		FATAL_MSG("[DSA::DSA] Invalid parameters!");
		throw std::invalid_argument("Invalid DSA parameters");
	}

	keys._private_key = generate_random(1, q - 1);
	keys._public_key = modular_pow(g, keys._private_key, p);

	DEBUG_MSG("[DSA::DSA] Initialized successfully. \npublic key: " + cpp_int_to_hex(keys._public_key) +
	          "\nprivate key: " + cpp_int_to_hex(keys._private_key));
}

DSASignature DSA::sign(const cpp_int& message_hash) {
	DSASignature signature;
	cpp_int k;

	while (signature._signature == 0) {
		k = generate_random(1, q - 1);
		signature._r = modular_pow(g, k, p) % q;

		if (signature._r == 0) {
			continue;
		}

		cpp_int k_inv = modular_pow(k, q - 2, q);
		signature._signature = (k_inv * (message_hash + keys._private_key * signature._r)) % q;
	}

	DEBUG_MSG("[DSA::sign] Signature generated successfully. \nr: " + cpp_int_to_hex(signature._r) +
	          "\ns: " + cpp_int_to_hex(signature._signature));

	return signature;
}

bool DSA::verify(const cpp_int& message_hash, const DSASignature& signature, const cpp_int& signer_public_key) {
	if (signature._r <= 0 || signature._r >= q || signature._signature <= 0 || signature._signature >= q) {
		DEBUG_MSG("[DSA::verify] Invalid signature values");
		return false;
	}

	cpp_int w = modular_pow(signature._signature, q - 2, q);
	cpp_int u1 = (message_hash * w) % q;
	cpp_int u2 = (signature._r * w) % q;
	cpp_int v = (modular_pow(g, u1, p) * modular_pow(signer_public_key, u2, p)) % p % q;

	bool is_valid = (v == signature._r);

	DEBUG_MSG("[DSA::verify] Signature verification " + std::string(is_valid ? "successful" : "failed"));

	return is_valid;
}

cpp_int DSA::get_public_key() const {
	return keys._public_key;
}

const KeyPair& DSA::get_keys() const {
	return keys;
}

} // namespace common
} // namespace crypto