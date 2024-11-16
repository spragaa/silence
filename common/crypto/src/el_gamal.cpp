#include "el_gamal.hpp"

namespace common {
namespace crypto {

ElGamal::ElGamal(const cpp_int& prime_modulus, const cpp_int& generator)
	: p(prime_modulus), g(generator) {

	if (!validate_parameters(p, g)) {
		throw std::invalid_argument("Invalid parameters");
	}

	keys.private_key = generate_random(2, p - 2);
	keys.public_key = modular_pow(g, keys.private_key, p);
}

cpp_int ElGamal::get_public_key() const {
	return keys.public_key;
}

const KeyPair& ElGamal::get_keys() const {
	return keys;
}

EncryptedMessage ElGamal::encrypt(const cpp_int& message, const cpp_int& recipient_public_key) {
	cpp_int k = generate_random(2, p - 2);

	EncryptedMessage encrypted;
	encrypted.c1 = modular_pow(g, k, p);
	encrypted.c2 = (message * modular_pow(recipient_public_key, k, p)) % p;

	return encrypted;
}

ElGamal::cpp_int ElGamal::decrypt(const EncryptedMessage& encrypted_message) {
	cpp_int s = modular_pow(encrypted_message.c1, keys.private_key, p);
	cpp_int s_inverse = modular_pow(s, p - 2, p);

	return (encrypted_message.c2 * s_inverse) % p;
}

} // namespace common
} // namespace crypto

// #include "el_gamal.hpp"
// #include "crypro_utils.hpp"
// #include <iostream>
// #include <iomanip>

// int main() {
// 	common::crypto::ElGamal::cpp_int p("0xFFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74"
// 	                                   "020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F1437"
// 	                                   "4FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
// 	                                   "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF05"
// 	                                   "98DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB"
// 	                                   "9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B"
// 	                                   "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF695581718"
// 	                                   "3995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF");

// 	common::crypto::ElGamal::cpp_int g("2");

// 	std::cout << "Initializing ElGamal with:\n";
// 	std::cout << "Prime (p): " << common::crypto::cpp_int_to_hex(p) << "\n";
// 	std::cout << "Generator (g): " << common::crypto::cpp_int_to_hex(g) << "\n\n";

// 	std::cout << "Generating keys for Alice and Bob...\n\n";
// 	common::crypto::ElGamal alice(p, g);
// 	common::crypto::ElGamal bob(p, g);

// 	std::cout << format_key_info("Alice's", alice.get_keys()) << "\n";
// 	std::cout << format_key_info("Bob's", bob.get_keys()) << "\n";

// 	std::cout << "Example 1: Small number encryption\n";
// 	common::crypto::ElGamal::cpp_int small_message("42");
// 	std::cout << "Original message: " << small_message << "\n";

// 	auto encrypted_small = alice.encrypt(small_message, bob.get_keys().public_key);
// 	std::cout << format_encrypted_message("Encrypted message", encrypted_small) << "\n";

// 	auto decrypted_small = bob.decrypt(encrypted_small);
// 	std::cout << "Decrypted message: " << decrypted_small << "\n";
// 	std::cout << "Decryption successful: " << (small_message == decrypted_small ? "Yes" : "No") << "\n\n";

// 	std::cout << "Example 2: AES-256 key encryption\n";
// 	common::crypto::ElGamal::cpp_int aes_key = common::crypto::hex_to_cpp_int("0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF");
// 	std::cout << "Original AES key: " << common::crypto::cpp_int_to_hex(aes_key) << "\n";

// 	auto encrypted_aes = alice.encrypt(aes_key, bob.get_keys().public_key);
// 	std::cout << format_encrypted_message("Encrypted AES key", encrypted_aes) << "\n";

// 	auto decrypted_aes = bob.decrypt(encrypted_aes);
// 	std::cout << "Decrypted AES key: " << common::crypto::cpp_int_to_hex(decrypted_aes) << "\n";
// 	std::cout << "Decryption successful: " << (aes_key == decrypted_aes ? "Yes" : "No") << "\n\n";

// 	std::cout << "Example 3: Message from Bob to Alice\n";
// 	common::crypto::ElGamal::cpp_int bob_message = common::crypto::hex_to_cpp_int("DEADBEEF");
// 	std::cout << "Bob's original message: " << common::crypto::cpp_int_to_hex(bob_message) << "\n";

// 	auto encrypted_bob = bob.encrypt(bob_message, alice.get_keys().public_key);
// 	std::cout << format_encrypted_message("Encrypted message", encrypted_bob) << "\n";

// 	auto decrypted_bob = alice.decrypt(encrypted_bob);
// 	std::cout << "Decrypted message: " << common::crypto::cpp_int_to_hex(decrypted_bob) << "\n";
// 	std::cout << "Decryption successful: " << (bob_message == decrypted_bob ? "Yes" : "No") << "\n";

// 	return 0;
// }
