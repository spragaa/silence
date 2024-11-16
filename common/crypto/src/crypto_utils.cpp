#include "crypto_utils.hpp"

#include <sstream>
#include <iomanip>

namespace common {
namespace crypto {

cpp_int hex_to_cpp_int(const std::string& hex) {
	cpp_int result;
	std::stringstream ss;
	ss << std::hex << hex;
	ss >> result;
	return result;
}

std::string cpp_int_to_hex(const cpp_int& num) {
	std::stringstream ss;
	ss << std::hex << num;
	return ss.str();
}

cpp_int find_valid_generator(const cpp_int& p) {
	std::vector<cpp_int> candidates = { 2, 3, 5, 7, 11,
		                                13, 17, 19, 23, 29,
		                                31, 37, 41, 43, 47,
		                                53, 59, 61, 67, 71,
		                                73, 79, 83, 89, 97 };

	cpp_int p_1 = p - 1;
	cpp_int q = p_1 / 2;

	for (const auto& g : candidates) {
		if (g >= p) break;

		cpp_int g2 = boost::multiprecision::powm(g, cpp_int(2), p);
		cpp_int gq = boost::multiprecision::powm(g, q, p);

		if (g2 != 1 && gq != 1) {
			return g;
		}
	}

	throw std::runtime_error("No valid generator found");
}

bool is_prime(const cpp_int& n, int iterations) {
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

bool validate_parameters(const cpp_int& p, const cpp_int& g) {
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

cpp_int find_generator(const cpp_int& p) {
	cpp_int q = (p - 1) / 2;

	for (cpp_int g = 2; g < p; g++) {
		if (modular_pow(g, 2, p) != 1 && modular_pow(g, q, p) != 1) {
			return g;
		}
	}

	throw std::runtime_error("Generator not found");
}

cpp_int generate_random(const cpp_int& min, const cpp_int& max) {
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

bool miller_rabin_test(const cpp_int& n, const cpp_int& a) {
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

cpp_int modular_pow(const cpp_int& base, const cpp_int& exponent, const cpp_int& modulus) {
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