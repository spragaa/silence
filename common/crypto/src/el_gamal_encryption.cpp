#include "el_gamal_encryption.hpp"

namespace common {
namespace crypto {

ElGamalEncryption::ElGamalEncryption(const cpp_int& prime_modulus, const cpp_int& generator)
	: _p(prime_modulus), _g(generator) {

	if (!validate_el_gamal_parameters(_p, _g)) {
		FATAL_MSG("[ElGamalEncryption::ElGamalEncryption] Invalid arguments!");
		throw std::invalid_argument("Invalid parameters");
	}

	_keys._private_key = generate_random(2, _p - 2);
	_keys._public_key = modular_pow(_g, _keys._private_key, _p);

	DEBUG_MSG("[ElGamalEncryption::ElGamalEncryption] Initialized successfully. \npublic key: " + cpp_int_to_hex(_keys._public_key) +
	          "\nprivate key: " + cpp_int_to_hex(_keys._private_key)
	          );
}

cpp_int ElGamalEncryption::get_public_key() const {
	return _keys._public_key;
}

const KeyPair& ElGamalEncryption::get_keys() const {
	return _keys;
}

EncryptedMessage ElGamalEncryption::encrypt(const cpp_int& message, const cpp_int& recipient_public_key) {
	cpp_int k = generate_random(2, _p - 2);

	EncryptedMessage encrypted;
	encrypted._c1 = modular_pow(_g, k, _p);
	encrypted._c2 = (message * modular_pow(recipient_public_key, k, _p)) % _p;

	DEBUG_MSG("[ElGamalEncryption::encrypt] Ecnryption successful. \nc1: "
	          + cpp_int_to_hex(encrypted._c1)
	          + "\nc2: " + cpp_int_to_hex(encrypted._c2)
	          );

	return encrypted;
}

ElGamalEncryption::cpp_int ElGamalEncryption::decrypt(const EncryptedMessage& encrypted_message) {
	cpp_int s = modular_pow(encrypted_message._c1, _keys._private_key, _p);
	cpp_int s_inverse = modular_pow(s, _p - 2, _p);

	auto decrypted = (encrypted_message._c2 * s_inverse) % _p;

	DEBUG_MSG("[ElGamalEncryption::decrypt] Decryption successful. Message: " + cpp_int_to_hex(decrypted));

	return decrypted;
}

} // namespace common
} // namespace crypto
