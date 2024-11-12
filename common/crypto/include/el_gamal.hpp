#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <random>
#include <array>
#include <vector>

class ElGamal {
public:
    struct KeyPair {
        boost::multiprecision::cpp_int private_key;
        boost::multiprecision::cpp_int public_key;
    };

    struct EncryptedMessage {
        boost::multiprecision::cpp_int c1;
        boost::multiprecision::cpp_int c2;
    };

private:
    boost::multiprecision::cpp_int p;
    boost::multiprecision::cpp_int g;
    KeyPair keys;

    boost::multiprecision::cpp_int generate_random(const boost::multiprecision::cpp_int& min, const boost::multiprecision::cpp_int& max) {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        
        boost::multiprecision::cpp_int range = max - min + 1;
        boost::multiprecision::cpp_int result;
        
        do {
            result = 0;
            for(size_t i = 0; i < sizeof(unsigned long long); ++i) {
                result = (result << 64) | gen();
            }
            result = result % range;
        } while (result < min);
        
        return result + min;
    }

public:
    ElGamal(const boost::multiprecision::cpp_int& prime_modulus, const boost::multiprecision::cpp_int& generator) 
        : p(prime_modulus), g(generator) {
        keys.private_key = generate_random(2, p - 2);
        keys.public_key = boost::multiprecision::powm(g, keys.private_key, p);
    }

    const KeyPair& get_keys() const { return keys; }

    EncryptedMessage encrypt(const boost::multiprecision::cpp_int& message, const boost::multiprecision::cpp_int& recipient_public_key) {
        boost::multiprecision::cpp_int k = generate_random(2, p - 2);
        
        EncryptedMessage encrypted;
        encrypted.c1 = boost::multiprecision::powm(g, k, p);
        encrypted.c2 = (message * boost::multiprecision::powm(recipient_public_key, k, p)) % p;
        
        return encrypted;
    }

    boost::multiprecision::cpp_int decrypt(const EncryptedMessage& encrypted_message) {
        boost::multiprecision::cpp_int s = boost::multiprecision::powm(encrypted_message.c1, keys.private_key, p);
        boost::multiprecision::cpp_int s_inverse = boost::multiprecision::powm(s, p - 2, p);
        
        return (encrypted_message.c2 * s_inverse) % p;
    }
};
