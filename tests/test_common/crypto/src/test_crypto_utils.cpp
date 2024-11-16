#include "crypto_utils.hpp"

#include <gtest/gtest.h>

namespace common::crypto {

using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_off>;

class CryptoUtilsTests : public ::testing::Test {};

TEST_F(CryptoUtilsTests, modular_pow_correctness) {
	cpp_int base(4);
	cpp_int exp(13);
	cpp_int mod(497);

	auto result = common::crypto::modular_pow(base, exp, mod);
	EXPECT_EQ(result, cpp_int(445));
}

TEST_F(CryptoUtilsTests, prime_validation) {
	EXPECT_TRUE(common::crypto::is_prime(251064135199));
	EXPECT_TRUE(common::crypto::is_prime(759869695727));
	EXPECT_TRUE(common::crypto::is_prime(167208199969));
	EXPECT_TRUE(common::crypto::is_prime(701906980781));

	EXPECT_FALSE(common::crypto::is_prime(4));
	EXPECT_FALSE(common::crypto::is_prime(100));
	EXPECT_FALSE(common::crypto::is_prime(999));

	EXPECT_FALSE(common::crypto::is_prime(0));
	EXPECT_FALSE(common::crypto::is_prime(1));
	EXPECT_TRUE(common::crypto::is_prime(2));
}

TEST_F(CryptoUtilsTests, miller_rabin_test_correctness) {
	cpp_int carmichael = 561;
	bool is_probably_prime = true;

	for (int i = 0; i < 20; i++) {
		cpp_int a = common::crypto::generate_random(2, carmichael - 2);
		if (!common::crypto::miller_rabin_test(carmichael, a)) {
			is_probably_prime = false;
			break;
		}
	}

	EXPECT_FALSE(is_probably_prime);
}

TEST_F(CryptoUtilsTests, generator_finding) {
	cpp_int small_p = 23;
	cpp_int found_g = common::crypto::find_generator(small_p);

	EXPECT_GT(found_g, cpp_int(1));
	EXPECT_LT(found_g, small_p);

	std::set<cpp_int> elements;
	cpp_int element = 1;

	for (cpp_int i = 0; i < small_p - 1; ++i) {
		element = (element * found_g) % small_p;
		elements.insert(element);
	}

	EXPECT_EQ(elements.size(), static_cast<size_t>(small_p - 1));
}

} // namespace common::crypto