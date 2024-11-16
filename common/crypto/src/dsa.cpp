#include "dsa.hpp"

namespace common {
namespace crypto {

DSA::DSA(const cpp_int& prime_modulus, const cpp_int& prime_divisor, const cpp_int& generator)
	: p(prime_modulus), q(prime_divisor), g(generator) {

	if (!validate_dsa_parameters(p, q, g)) {
		FATAL_MSG("[DSA::DSA] Invalid parameters!");
		throw std::invalid_argument("Invalid DSA parameters");
	}

	keys.private_key = generate_random(1, q - 1);
	keys.public_key = modular_pow(g, keys.private_key, p);

	DEBUG_MSG("[DSA::DSA] Initialized successfully. \npublic key: " + cpp_int_to_hex(keys.public_key) +
	          "\nprivate key: " + cpp_int_to_hex(keys.private_key));
}

DSASignature DSA::sign(const cpp_int& message_hash) {
	DSASignature signature;
	cpp_int k;

	while (signature.signature == 0) {
		k = generate_random(1, q - 1);
		signature.r = modular_pow(g, k, p) % q;

		if (signature.r == 0) {
			continue;
		}

		cpp_int k_inv = modular_pow(k, q - 2, q);
		signature.signature = (k_inv * (message_hash + keys.private_key * signature.r)) % q;
	}

	DEBUG_MSG("[DSA::sign] Signature generated successfully. \nr: " + cpp_int_to_hex(signature.r) +
	          "\ns: " + cpp_int_to_hex(signature.signature));

	return signature;
}

bool DSA::verify(const cpp_int& message_hash, const DSASignature& signature, const cpp_int& signer_public_key) {
	if (signature.r <= 0 || signature.r >= q || signature.signature <= 0 || signature.signature >= q) {
		DEBUG_MSG("[DSA::verify] Invalid signature values");
		return false;
	}

	cpp_int w = modular_pow(signature.signature, q - 2, q);
	cpp_int u1 = (message_hash * w) % q;
	cpp_int u2 = (signature.r * w) % q;
	cpp_int v = (modular_pow(g, u1, p) * modular_pow(signer_public_key, u2, p)) % p % q;

	bool is_valid = (v == signature.r);

	DEBUG_MSG("[DSA::verify] Signature verification " + std::string(is_valid ? "successful" : "failed"));

	return is_valid;
}

cpp_int DSA::get_public_key() const {
	return keys.public_key;
}

const KeyPair& DSA::get_keys() const {
	return keys;
}

} // namespace common
} // namespace crypto