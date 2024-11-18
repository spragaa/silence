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

// #include "el_gamal_encryption.hpp"
// #include "crypro_utils.hpp"
// #include <iostream>
// #include <iomanip>

// int main() {
// 	common::crypto::ElGamalEncryption::cpp_int _p("0xFFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74"
// 	                                   "020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F1437"
// 	                                   "4FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
// 	                                   "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF05"
// 	                                   "98DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB"
// 	                                   "9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B"
// 	                                   "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF695581718"
// 	                                   "3995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF");

// 	common::crypto::ElGamalEncryption::cpp_int _g("2");

// 	std::cout << "Initializing ElGamalEncryption with:\n";
// 	std::cout << "Prime (_p): " << common::crypto::cpp_int_to_hex(_p) << "\n";
// 	std::cout << "Generator (_g): " << common::crypto::cpp_int_to_hex(_g) << "\n\n";

// 	std::cout << "Generating _keys for Alice and Bob...\n\n";
// 	common::crypto::ElGamalEncryption alice(_p, _g);
// 	common::crypto::ElGamalEncryption bob(_p, _g);

// 	std::cout << format_key_info("Alice's", alice.get_keys()) << "\n";
// 	std::cout << format_key_info("Bob's", bob.get_keys()) << "\n";

// 	std::cout << "Example 1: Small number encryption\n";
// 	common::crypto::ElGamalEncryption::cpp_int small_message("42");
// 	std::cout << "Original message: " << small_message << "\n";

// 	auto encrypted_small = alice.encrypt(small_message, bob.get_keys()._public_key);
// 	std::cout << format_encrypted_message("Encrypted message", encrypted_small) << "\n";

// 	auto decrypted_small = bob.decrypt(encrypted_small);
// 	std::cout << "Decrypted message: " << decrypted_small << "\n";
// 	std::cout << "Decryption successful: " << (small_message == decrypted_small ? "Yes" : "No") << "\n\n";

// 	std::cout << "Example 2: AES-256 key encryption\n";
// 	common::crypto::ElGamalEncryption::cpp_int aes_key = common::crypto::hex_to_cpp_int("0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF");
// 	std::cout << "Original AES key: " << common::crypto::cpp_int_to_hex(aes_key) << "\n";

// 	auto encrypted_aes = alice.encrypt(aes_key, bob.get_keys()._public_key);
// 	std::cout << format_encrypted_message("Encrypted AES key", encrypted_aes) << "\n";

// 	auto decrypted_aes = bob.decrypt(encrypted_aes);
// 	std::cout << "Decrypted AES key: " << common::crypto::cpp_int_to_hex(decrypted_aes) << "\n";
// 	std::cout << "Decryption successful: " << (aes_key == decrypted_aes ? "Yes" : "No") << "\n\n";

// 	std::cout << "Example 3: Message from Bob to Alice\n";
// 	common::crypto::ElGamalEncryption::cpp_int bob_message = common::crypto::hex_to_cpp_int("DEADBEEF");
// 	std::cout << "Bob's original message: " << common::crypto::cpp_int_to_hex(bob_message) << "\n";

// 	auto encrypted_bob = bob.encrypt(bob_message, alice.get_keys()._public_key);
// 	std::cout << format_encrypted_message("Encrypted message", encrypted_bob) << "\n";

// 	auto decrypted_bob = alice.decrypt(encrypted_bob);
// 	std::cout << "Decrypted message: " << common::crypto::cpp_int_to_hex(decrypted_bob) << "\n";
// 	std::cout << "Decryption successful: " << (bob_message == decrypted_bob ? "Yes" : "No") << "\n";

// 	return 0;
// }
