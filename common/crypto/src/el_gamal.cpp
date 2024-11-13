#include "el_gamal.hpp"

namespace common {
namespace crypto {

ElGamal::ElGamal(const cpp_int& prime_modulus, const cpp_int& generator) 
    : p(prime_modulus), g(generator) {
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
    
    do {
        result = 0;
        for(size_t i = 0; i < sizeof(unsigned long long); ++i) {
            result = (result << 64) | gen();
        }
        result = result % range;
    } while (result < min);
    
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

} // namespace common
} // namespace crypto