#include "dsa.hpp"
#include "crypto_utils.hpp"

#include <gtest/gtest.h>

namespace common::crypto {

class TestDSA : public DSA {
public:
    TestDSA(const cpp_int& p, const cpp_int& q, const cpp_int& g) 
        : DSA(p, q, g) {}
    
    using DSA::get_keys;
};

class DSATests : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        p = hex_to_cpp_int(
            "8df2a494492276aa3d25759bb06869cbeac0d83afb8d0cf7cbb8324f0d7882e5"
            "d0762fc5b7210eafc2e9adac32ab7aac49693dfbf83724c2ec0736ee31c80291");
        
        q = hex_to_cpp_int("c773218c737ec8ee993b4f2ded30f48edace915f");
        
        g = hex_to_cpp_int(
            "626d027839ea0a13413163a55b4cb500299d5522956cefcb3bff10f399ce2c2e"
            "71cb9de5fa24babf58e5b79521925c9cc42e9f6f464b088cc572af53e6d78802");

        alice = std::make_unique<TestDSA>(p, q, g);
        bob = std::make_unique<TestDSA>(p, q, g);
    }

    static void TearDownTestSuite() {
        alice.reset();
        bob.reset();
    }

    static TestDSA::cpp_int p;
    static TestDSA::cpp_int q;
    static TestDSA::cpp_int g;
    static std::unique_ptr<TestDSA> alice;
    static std::unique_ptr<TestDSA> bob;
};

TestDSA::cpp_int DSATests::p;
TestDSA::cpp_int DSATests::q;
TestDSA::cpp_int DSATests::g;
std::unique_ptr<TestDSA> DSATests::alice;
std::unique_ptr<TestDSA> DSATests::bob;

TEST_F(DSATests, key_generation) {
    const auto& keys = alice->get_keys();

    EXPECT_GT(keys._private_key, 1);
    EXPECT_LT(keys._private_key, q);
    EXPECT_GT(keys._public_key, 1);
    EXPECT_LT(keys._public_key, p);
}

TEST_F(DSATests, signature_verification) {
    cpp_int message_hash = hex_to_cpp_int("0123456789abcdef0123456789abcdef01234567");
    
    auto signature = alice->sign(message_hash);
    bool is_valid = bob->verify(message_hash, signature, alice->get_public_key());
    
    EXPECT_TRUE(is_valid);
}

TEST_F(DSATests, invalid_message_fails_verification) {
    cpp_int message_hash = hex_to_cpp_int("0123456789abcdef0123456789abcdef01234567");
    cpp_int different_hash = hex_to_cpp_int("0123456789abcdef0123456789abcdef01234568");
    
    auto signature = alice->sign(message_hash);
    bool is_valid = bob->verify(different_hash, signature, alice->get_public_key());
    
    EXPECT_FALSE(is_valid);
}

TEST_F(DSATests, signature_randomness) {
    cpp_int message_hash = hex_to_cpp_int("0123456789abcdef0123456789abcdef01234567");
    
    auto signature1 = alice->sign(message_hash);
    auto signature2 = alice->sign(message_hash);
    
    EXPECT_NE(signature1._r, signature2._r);
    EXPECT_NE(signature1._signature, signature2._signature);
    
    EXPECT_TRUE(bob->verify(message_hash, signature1, alice->get_public_key()));
    EXPECT_TRUE(bob->verify(message_hash, signature2, alice->get_public_key()));
}

TEST_F(DSATests, cross_verification) {
    cpp_int message_hash = hex_to_cpp_int("0123456789abcdef0123456789abcdef01234567");
    
    auto alice_signature = alice->sign(message_hash);
    EXPECT_TRUE(bob->verify(message_hash, alice_signature, alice->get_public_key()));
    
    auto bob_signature = bob->sign(message_hash);
    EXPECT_TRUE(alice->verify(message_hash, bob_signature, bob->get_public_key()));
}

TEST_F(DSATests, multiple_message_signing) {
    std::vector<cpp_int> message_hashes = {
        hex_to_cpp_int("0123456789abcdef0123456789abcdef01234567"),
        hex_to_cpp_int("fedcba9876543210fedcba9876543210fedcba98"),
        hex_to_cpp_int("123456789abcdef0123456789abcdef012345678")
    };
    
    for (const auto& hash : message_hashes) {
        auto signature = alice->sign(hash);
        EXPECT_TRUE(bob->verify(hash, signature, alice->get_public_key()));
    }
}

TEST_F(DSATests, invalid_signature_components) {
    cpp_int message_hash = hex_to_cpp_int("0123456789abcdef0123456789abcdef01234567");
    auto signature = alice->sign(message_hash);
    
    DSASignature invalid_sig1 = {cpp_int(0), signature._signature};
    EXPECT_FALSE(bob->verify(message_hash, invalid_sig1, alice->get_public_key()));
    
    DSASignature invalid_sig2 = {signature._r, cpp_int(0)};
    EXPECT_FALSE(bob->verify(message_hash, invalid_sig2, alice->get_public_key()));
    
    DSASignature invalid_sig3 = {q, signature._signature};
    EXPECT_FALSE(bob->verify(message_hash, invalid_sig3, alice->get_public_key()));
}

TEST_F(DSATests, parameter_validation) {
    EXPECT_THROW(TestDSA(cpp_int(4), q, g), std::invalid_argument);
    EXPECT_THROW(TestDSA(p, cpp_int(4), g), std::invalid_argument);
    EXPECT_THROW(TestDSA(p, q, p), std::invalid_argument);

    cpp_int wrong_q = hex_to_cpp_int("c773218c737ec8ee993b4f2ded30f48edace9150");
    EXPECT_THROW(TestDSA(p, wrong_q, g), std::invalid_argument);
}

TEST_F(DSATests, wrong_public_key_verification) {
    cpp_int message_hash = hex_to_cpp_int("0123456789abcdef0123456789abcdef01234567");
    auto signature = alice->sign(message_hash);
    
    EXPECT_FALSE(bob->verify(message_hash, signature, bob->get_public_key()));
}

} // namespace common::crypto