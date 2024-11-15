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

const KeyPair& ElGamal::get_keys() const {
	return keys;
}

ElGamal::cpp_int ElGamal::generate_random(const cpp_int& min, const cpp_int& max) {
	std::random_device rd;
	std::mt19937_64 gen(rd());

	cpp_int range = max - min + 1;
	cpp_int result;

	while(result < min) {
		result = 0;
		for(size_t i = 0; i < sizeof(unsigned long long); ++i) {
			result = (result << 64) | gen();
		}
		result = result % range;
	}

	return result + min;
}

ElGamal::cpp_int ElGamal::modular_pow(const cpp_int& base, const cpp_int& exponent, const cpp_int& modulus) {
	if (modulus == 1) return 0;

	cpp_int result = 1;
	cpp_int b = base % modulus;
	cpp_int exp = exponent;

	while (exp > 0) {
		if (exp % 2 == 1)
			result = (result * b) % modulus;
		b = (b * b) % modulus;
		exp /= 2;
	}

	return result;
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

bool ElGamal::miller_rabin_test(const cpp_int& n, const cpp_int& a) {
    cpp_int d = n - 1;
    cpp_int r = 0;
    
    while (d % 2 == 0) {
        d /= 2;
        r += 1;
    }
    
    cpp_int x = modular_pow(a, d, n);
    if (x == 1 || x == n - 1) return true;
    
    for (cpp_int i = 0; i < r - 1; i++) {
        x = (x * x) % n;
        if (x == n - 1) return true;
        if (x == 1) return false;
    }
    
    return false;
}

bool ElGamal::is_prime(const cpp_int& n, int iterations) {
    if (n <= 1 || n == 4) return false;
    if (n <= 3) return true;
    
    cpp_int small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
    for (const auto& p : small_primes) {
        if (n == p) return true;
        if (n % p == 0) return false;
    }
    
    std::random_device rd;
    std::mt19937_64 gen(rd());
    
    for (int i = 0; i < iterations; i++) {
        cpp_int a = generate_random(2, n - 2);
        if (!miller_rabin_test(n, a))
            return false;
    }
    
    return true;
}

bool ElGamal::validate_parameters(const cpp_int& p, const cpp_int& g) {
    if (!is_prime(p)) {
        throw std::invalid_argument("The modulus p must be prime");
    }
    
    if (g < 2 || g >= p) {
        throw std::invalid_argument("Generator g must be in range [2, p-1]");
    }
    
    cpp_int factors[] = {2, (p-1)/2};
    for (const auto& factor : factors) {
        if (modular_pow(g, (p-1)/factor, p) == 1) {
            throw std::invalid_argument("g is not a primitive root modulo p");
        }
    }
    
    return true;
}

cpp_int ElGamal::generate_safe_prime(size_t bits) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    
    while (true) {
        cpp_int q = generate_random(cpp_int(1) << (bits-2), cpp_int(1) << (bits-1));
        
        if (q % 2 == 0) q += 1;
        
        if (!is_prime(q)) continue;
        
        cpp_int p = 2 * q + 1;
        
        if (is_prime(p)) {
            return p;
        }
    }
}

cpp_int ElGamal::find_generator(const cpp_int& p) {
    cpp_int q = (p - 1) / 2;
    
    for (cpp_int g = 2; g < p; g++) {
        if (modular_pow(g, 2, p) != 1 && modular_pow(g, q, p) != 1) {
            return g;
        }
    }
    
    throw std::runtime_error("Generator not found");
}

std::string format_key_info(const std::string& label, const common::crypto::KeyPair& keys) {
	std::stringstream ss;
	ss << label << " Keys:\n";
	ss << "  Private: " << common::crypto::cpp_int_to_hex(keys.private_key) << "\n";
	ss << "  Public:  " << common::crypto::cpp_int_to_hex(keys.public_key) << "\n";
	return ss.str();
}

std::string format_encrypted_message(const std::string& label, const common::crypto::EncryptedMessage& msg) {
	std::stringstream ss;
	ss << label << ":\n";
	ss << "  C1: " << common::crypto::cpp_int_to_hex(msg.c1) << "\n";
	ss << "  C2: " << common::crypto::cpp_int_to_hex(msg.c2) << "\n";
	return ss.str();
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
