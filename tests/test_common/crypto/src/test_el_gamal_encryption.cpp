#include "el_gamal_encryption.hpp"
#include "crypto_utils.hpp"

#include <gtest/gtest.h>

namespace common::crypto {

class TestElGamalEncryption : public ElGamalEncryption {
public:
	TestElGamalEncryption(const cpp_int& p, const cpp_int& g) : ElGamalEncryption(p, g) {
	}

	using ElGamalEncryption::get_keys;
};

class ElGamalEncryptionTests : public ::testing::Test {
protected:

	static void SetUpTestSuite() {
		p = hex_to_cpp_int(
			"ffffffffffffffffc90fdaa22168c234c4c6628b80dc1cd129024e088a67cc74"
			"020bbea63b139b22514a08798e3404ddef9519b3cd3a431b302b0a6df25f1437"
			"4fe1356d6d51c245e485b576625e7ec6f44c42e9a637ed6b0bff5cb6f406b7ed"
			"ee386bfb5a899fa5ae9f24117c4b1fe649286651ece45b3dc2007cb8a163bf05"
			"98da48361c55d39a69163fa8fd24cf5f83655d23dca3ad961c62f356208552bb"
			"9ed529077096966d670c354e4abc9804f1746c08ca18217c32905e462e36ce3b"
			"e39e772c180e86039b2783a2ec07a28fb5c55df06f4c52c9de2bcbf695581718"
			"3995497cea956ae515d2261898fa051015728e5a8aacaa68ffffffffffffffff"
			);
		g = TestElGamalEncryption::cpp_int(11); // 11 is safe, if you want to change 'p', use find_valid_generator to find new safe 'g'
		// g = find_valid_generator(p);
		// std::cout << "Using generator: " << g << std::endl;

		alice = std::make_unique<TestElGamalEncryption>(p, g);
		bob = std::make_unique<TestElGamalEncryption>(p, g);
	}

	static void TearDownTestSuite() {
		alice.reset();
		bob.reset();
	}

	static TestElGamalEncryption::cpp_int p;
	static TestElGamalEncryption::cpp_int g;
	static std::unique_ptr<TestElGamalEncryption> alice;
	static std::unique_ptr<TestElGamalEncryption> bob;
};

TestElGamalEncryption::cpp_int ElGamalEncryptionTests::p;
TestElGamalEncryption::cpp_int ElGamalEncryptionTests::g;
std::unique_ptr<TestElGamalEncryption> ElGamalEncryptionTests::alice;
std::unique_ptr<TestElGamalEncryption> ElGamalEncryptionTests::bob;


TEST_F(ElGamalEncryptionTests, key_generation) {
	const auto& keys = alice->get_keys();

	EXPECT_GT(keys._private_key, 1);
	EXPECT_LT(keys._private_key, p - 1);
	EXPECT_GT(keys._public_key, 1);
	EXPECT_LT(keys._public_key, p);
}

TEST_F(ElGamalEncryptionTests, encrypt_decrypt_small_message) {
	common::crypto::TestElGamalEncryption::cpp_int message(42);

	auto encrypted = alice->encrypt(message, bob->get_keys()._public_key);
	auto decrypted = bob->decrypt(encrypted);

	EXPECT_EQ(message, decrypted);
}

TEST_F(ElGamalEncryptionTests, encrypt_decrypt_large_message) {
	auto message = common::crypto::hex_to_cpp_int(
		"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
		);

	auto encrypted = alice->encrypt(message, bob->get_keys()._public_key);
	auto decrypted = bob->decrypt(encrypted);

	EXPECT_EQ(message, decrypted);
}

TEST_F(ElGamalEncryptionTests, bidirectional_communication) {
	common::crypto::TestElGamalEncryption::cpp_int message1(123);
	auto encrypted1 = alice->encrypt(message1, bob->get_keys()._public_key);
	auto decrypted1 = bob->decrypt(encrypted1);
	EXPECT_EQ(message1, decrypted1);

	common::crypto::TestElGamalEncryption::cpp_int message2(456);
	auto encrypted2 = bob->encrypt(message2, alice->get_keys()._public_key);
	auto decrypted2 = alice->decrypt(encrypted2);
	EXPECT_EQ(message2, decrypted2);
}

TEST_F(ElGamalEncryptionTests, multiple_messages_encryption) {
	std::vector<common::crypto::ElGamalEncryption::cpp_int> messages = {
		common::crypto::TestElGamalEncryption::cpp_int(1),
		common::crypto::TestElGamalEncryption::cpp_int(100),
		common::crypto::TestElGamalEncryption::cpp_int(65535),
		common::crypto::hex_to_cpp_int("DEADBEEF")
	};

	for (const auto& message : messages) {
		auto encrypted = alice->encrypt(message, bob->get_keys()._public_key);
		auto decrypted = bob->decrypt(encrypted);
		EXPECT_EQ(message, decrypted);
	}
}

TEST_F(ElGamalEncryptionTests, edge_cases) {
	common::crypto::TestElGamalEncryption::cpp_int message1(1);
	auto encrypted1 = alice->encrypt(message1, bob->get_keys()._public_key);
	auto decrypted1 = bob->decrypt(encrypted1);
	EXPECT_EQ(message1, decrypted1);

	common::crypto::TestElGamalEncryption::cpp_int message2 = p - 1;
	auto encrypted2 = alice->encrypt(message2, bob->get_keys()._public_key);
	auto decrypted2 = bob->decrypt(encrypted2);
	EXPECT_EQ(message2, decrypted2);
}

TEST_F(ElGamalEncryptionTests, randomness_in_encryption) {
	common::crypto::ElGamalEncryption::cpp_int message(42);

	auto encrypted1 = alice->encrypt(message, bob->get_keys()._public_key);
	auto encrypted2 = alice->encrypt(message, bob->get_keys()._public_key);

	EXPECT_NE(encrypted1._c1, encrypted2._c1);
	EXPECT_NE(encrypted1._c2, encrypted2._c2);

	EXPECT_EQ(bob->decrypt(encrypted1), bob->decrypt(encrypted2));
}

TEST_F(ElGamalEncryptionTests, demonstrate_prime_size_effects) {
	cpp_int small_p = 23;
	cpp_int g = 11;

	common::crypto::TestElGamalEncryption small_prime_system(small_p, g);
	common::crypto::TestElGamalEncryption large_prime_system(p, g);

	cpp_int message(1000);

	auto encrypted_small = small_prime_system.encrypt(message,
	                                                  small_prime_system.get_keys()._public_key);
	auto decrypted_small = small_prime_system.decrypt(encrypted_small);
	EXPECT_NE(message, decrypted_small);

	auto encrypted_large = large_prime_system.encrypt(message,
	                                                  large_prime_system.get_keys()._public_key);
	auto decrypted_large = large_prime_system.decrypt(encrypted_large);
	EXPECT_EQ(message, decrypted_large);
}

TEST_F(ElGamalEncryptionTests, parameter_validation) {
	EXPECT_NO_THROW(validate_el_gamal_parameters(p, g));

	cpp_int non_prime = p * 2;
	EXPECT_THROW(validate_el_gamal_parameters(non_prime, g), std::invalid_argument);

	EXPECT_THROW(validate_el_gamal_parameters(p, p), std::invalid_argument);

	EXPECT_THROW(validate_el_gamal_parameters(p, cpp_int(1)), std::invalid_argument);
}

// TEST_F(ElGamalEncryptionTests, key_generation_with_different_primes) {
//     std::vector<size_t> bit_sizes = {512, 768, 1024};

//     for (size_t bits : bit_sizes) {
//         TestElGamalEncryption el_gamal_encryption(p, g);
//         cpp_int test_p = el_gamal_encryption.generate_safe_prime(bits);
//         cpp_int test_g = find_generator(test_p);

//         TestElGamalEncryption test_system(test_p, test_g);
//         const auto& keys = test_system.get_keys();

//         EXPECT_GT(keys._private_key, cpp_int(1));
//         EXPECT_LT(keys._private_key, test_p - 1);
//         EXPECT_GT(keys._public_key, cpp_int(1));
//         EXPECT_LT(keys._public_key, test_p);
//     }
// }

// TEST_F(ElGamalEncryptionTests, encryption_with_different_prime_sizes) {
//     std::vector<size_t> bit_sizes = {512, 768, 1024};
//     cpp_int message(42);

//     for (size_t bits : bit_sizes) {
//         TestElGamalEncryption el_gamal_encryption(p, g);
//         cpp_int test_p = el_gamal_encryption.generate_safe_prime(bits);
//         cpp_int test_g = find_generator(test_p);

//         TestElGamalEncryption alice(test_p, test_g);
//         TestElGamalEncryption bob(test_p, test_g);

//         auto encrypted = alice.encrypt(message, bob.get_keys()._public_key);
//         auto decrypted = bob.decrypt(encrypted);

//         EXPECT_EQ(message, decrypted);
//     }
// }

TEST_F(ElGamalEncryptionTests, invalid_parameter_construction) {
	EXPECT_THROW(TestElGamalEncryption(cpp_int(4), cpp_int(2)), std::invalid_argument);
	EXPECT_THROW(TestElGamalEncryption(p, p), std::invalid_argument);
	EXPECT_THROW(TestElGamalEncryption(p, cpp_int(1)), std::invalid_argument);
}

} // namespace common::crypto
