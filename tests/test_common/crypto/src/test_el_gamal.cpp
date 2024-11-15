#include "el_gamal.hpp"
#include "crypto_utils.hpp"

#include <gtest/gtest.h>

namespace common::crypto {
    
class TestElGamal : public ElGamal {
public:

    TestElGamal(const cpp_int& p, const cpp_int& g) : ElGamal(p, g) {}
    
    using ElGamal::generate_random;
    using ElGamal::modular_pow;
};

class ElGamalTest : public ::testing::Test {
protected:
    common::crypto::TestElGamal::cpp_int p;
    common::crypto::TestElGamal::cpp_int g;
    
    void SetUp() override {
        p = common::crypto::hex_to_cpp_int(
            "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74"
            "020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F1437"
            "4FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
            "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF05"
            "98DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB"
            "9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B"
            "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF695581718"
            "3995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF"
        );
        g = common::crypto::TestElGamal::cpp_int(2);
    }
};

TEST_F(ElGamalTest, key_generation) {
    common::crypto::TestElGamal el_gamal(p, g);
    const auto& keys = el_gamal.get_keys();
    
    EXPECT_GT(keys.private_key, 1);
    EXPECT_LT(keys.private_key, p - 1);
    EXPECT_GT(keys.public_key, 1);
    EXPECT_LT(keys.public_key, p);
}

TEST_F(ElGamalTest, encrypt_decrypt_small_message) {
    common::crypto::TestElGamal alice(p, g);
    common::crypto::TestElGamal bob(p, g);
    
    common::crypto::TestElGamal::cpp_int message(42);
    
    auto encrypted = alice.encrypt(message, bob.get_keys().public_key);
    auto decrypted = bob.decrypt(encrypted);
    
    EXPECT_EQ(message, decrypted);
}

TEST_F(ElGamalTest, encrypt_decrypt_large_message) {
    common::crypto::TestElGamal alice(p, g);
    common::crypto::TestElGamal bob(p, g);
    
    auto message = common::crypto::hex_to_cpp_int(
        "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
    );
    
    auto encrypted = alice.encrypt(message, bob.get_keys().public_key);
    auto decrypted = bob.decrypt(encrypted);
    
    EXPECT_EQ(message, decrypted);
}

TEST_F(ElGamalTest, bidirectional_communication) {
    common::crypto::TestElGamal alice(p, g);
    common::crypto::TestElGamal bob(p, g);
    
    common::crypto::TestElGamal::cpp_int message1(123);
    auto encrypted1 = alice.encrypt(message1, bob.get_keys().public_key);
    auto decrypted1 = bob.decrypt(encrypted1);
    EXPECT_EQ(message1, decrypted1);
    
    common::crypto::TestElGamal::cpp_int message2(456);
    auto encrypted2 = bob.encrypt(message2, alice.get_keys().public_key);
    auto decrypted2 = alice.decrypt(encrypted2);
    EXPECT_EQ(message2, decrypted2);
}

TEST_F(ElGamalTest, multiple_messages_encryption) {
    common::crypto::TestElGamal alice(p, g);
    common::crypto::TestElGamal bob(p, g);
    
    std::vector<common::crypto::ElGamal::cpp_int> messages = {
        common::crypto::TestElGamal::cpp_int(1),
        common::crypto::TestElGamal::cpp_int(100),
        common::crypto::TestElGamal::cpp_int(65535),
        common::crypto::hex_to_cpp_int("DEADBEEF")
    };
    
    for (const auto& message : messages) {
        auto encrypted = alice.encrypt(message, bob.get_keys().public_key);
        auto decrypted = bob.decrypt(encrypted);
        EXPECT_EQ(message, decrypted);
    }
}

TEST_F(ElGamalTest, modular_pow_correctness) {
    common::crypto::TestElGamal el_gamal(p, g);
    
    common::crypto::TestElGamal::cpp_int base(4);
    common::crypto::TestElGamal::cpp_int exp(13);
    common::crypto::TestElGamal::cpp_int mod(497);
    
    auto result = el_gamal.modular_pow(base, exp, mod);
    EXPECT_EQ(result, common::crypto::ElGamal::cpp_int(445));
}

TEST_F(ElGamalTest, edge_cases) {
    common::crypto::TestElGamal alice(p, g);
    common::crypto::TestElGamal bob(p, g);
    
    common::crypto::TestElGamal::cpp_int message1(1);
    auto encrypted1 = alice.encrypt(message1, bob.get_keys().public_key);
    auto decrypted1 = bob.decrypt(encrypted1);
    EXPECT_EQ(message1, decrypted1);
    
    common::crypto::TestElGamal::cpp_int message2 = p - 1;
    auto encrypted2 = alice.encrypt(message2, bob.get_keys().public_key);
    auto decrypted2 = bob.decrypt(encrypted2);
    EXPECT_EQ(message2, decrypted2);
}

TEST_F(ElGamalTest, randomness_in_encryption) {
    common::crypto::ElGamal alice(p, g);
    common::crypto::ElGamal bob(p, g);
    
    common::crypto::ElGamal::cpp_int message(42);
    
    auto encrypted1 = alice.encrypt(message, bob.get_keys().public_key);
    auto encrypted2 = alice.encrypt(message, bob.get_keys().public_key);
    
    EXPECT_NE(encrypted1.c1, encrypted2.c1);
    EXPECT_NE(encrypted1.c2, encrypted2.c2);
    
    EXPECT_EQ(bob.decrypt(encrypted1), bob.decrypt(encrypted2));
}

TEST_F(ElGamalTest, key_pair_formatting) {
    common::crypto::ElGamal el_gamal(p, g);
    const auto& keys = el_gamal.get_keys();
    
    std::string formatted = format_key_info("Test", keys);
    EXPECT_FALSE(formatted.empty());
    EXPECT_NE(formatted.find("Private:"), std::string::npos);
    EXPECT_NE(formatted.find("Public:"), std::string::npos);
}

TEST_F(ElGamalTest, encrypted_message_formatting) {
    common::crypto::ElGamal alice(p, g);
    common::crypto::ElGamal bob(p, g);
    
    auto encrypted = alice.encrypt(
        common::crypto::ElGamal::cpp_int(42),
        bob.get_keys().public_key
    );
    
    std::string formatted = format_encrypted_message("Test", encrypted);
    EXPECT_FALSE(formatted.empty());
    EXPECT_NE(formatted.find("C1:"), std::string::npos);
    EXPECT_NE(formatted.find("C2:"), std::string::npos);
}

TEST_F(ElGamalTest, demonstrate_prime_size_effects) {
    cpp_int small_p = 23;
    cpp_int g = 2;
    
    common::crypto::TestElGamal small_prime_system(small_p, g);
    common::crypto::TestElGamal large_prime_system(p, g);
    
    cpp_int message(1000);
    
    auto encrypted_small = small_prime_system.encrypt(message, 
        small_prime_system.get_keys().public_key);
    auto decrypted_small = small_prime_system.decrypt(encrypted_small);
    EXPECT_NE(message, decrypted_small);
    
    auto encrypted_large = large_prime_system.encrypt(message, 
        large_prime_system.get_keys().public_key);
    auto decrypted_large = large_prime_system.decrypt(encrypted_large);
    EXPECT_EQ(message, decrypted_large);
}

} // namespace common::crypto