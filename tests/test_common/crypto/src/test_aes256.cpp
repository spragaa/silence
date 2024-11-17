#include "aes256.hpp"

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <random>

namespace common::crypto {

constexpr std::array<char, 62> alphabet = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
	'y', 'z'
};

class AESTests : public ::testing::Test, public AES256 {
protected:
	void SetUp() override {
		std::fill(test_key.begin(), test_key.end(), 0x42);
		test_state.fill(0x42);
		test_word = {0x42, 0x43, 0x44, 0x45};
		
		aes.set_key(test_key);
	}

	std::string generate_random_string(const int& len) {
		std::random_device random_device;
		std::mt19937 generator(random_device());
		std::uniform_int_distribution<> distribution(0, alphabet.size() - 1);

		std::string str;

		for (std::size_t i = 0; i < len; ++i) {
			str += alphabet[distribution(generator)];
		}

		return str;
	};

protected:
    AES256 aes;
	std::array<uint8_t, 16> test_state;
	std::array<uint8_t, 32> test_key;
	std::array<uint8_t, 4> test_word;
	
};

TEST_F(AESTests, galois_multiplication_test) {
	EXPECT_EQ(aes.gmul(0x57, 0x83), 0xc1);
	EXPECT_EQ(aes.gmul(0x57, 0x13), 0xfe);
	EXPECT_EQ(aes.gmul(0x57, 0x00), 0x00);
	EXPECT_EQ(aes.gmul(0x00, 0x57), 0x00);
	EXPECT_EQ(aes.gmul(0x01, 0x57), 0x57);
}

TEST_F(AESTests, sub_bytes_test) {
	auto result = aes.sub_bytes(test_state);
	EXPECT_EQ(result[0], AES256::sbox[0x42]);

	auto inverse = aes.inv_sub_bytes(result);
	EXPECT_EQ(inverse, test_state);
}

TEST_F(AESTests, shift_rows_test) {
	for(size_t i = 0; i < 16; i++) {
		test_state[i] = i;
	}

	/* State:
	 * 0  4  8  12
	 * 1  5  9  13
	 * 2  6  10 14
	 * 3  7  11 15
	 */

	auto shifted = aes.shift_rows(test_state);

	/* After shift_rows, should be:
	 * 0  4  8  12  -- no shift
	 * 5  9  13 1   -- shift by 1
	 * 10 14 2  6   -- shift by 2
	 * 15 3  7  11  -- shift by 3
	 */

	EXPECT_EQ(shifted[0], test_state[0]);
	EXPECT_EQ(shifted[4], test_state[4]);
	EXPECT_EQ(shifted[8], test_state[8]);
	EXPECT_EQ(shifted[12], test_state[12]);

	EXPECT_EQ(shifted[1], test_state[5]);
	EXPECT_EQ(shifted[5], test_state[9]);
	EXPECT_EQ(shifted[9], test_state[13]);
	EXPECT_EQ(shifted[13], test_state[1]);

	EXPECT_EQ(shifted[2], test_state[10]);
	EXPECT_EQ(shifted[6], test_state[14]);
	EXPECT_EQ(shifted[10], test_state[2]);
	EXPECT_EQ(shifted[14], test_state[6]);

	EXPECT_EQ(shifted[3], test_state[15]);
	EXPECT_EQ(shifted[7], test_state[3]);
	EXPECT_EQ(shifted[11], test_state[7]);
	EXPECT_EQ(shifted[15], test_state[11]);

	auto inverse = aes.inv_shift_rows(shifted);
	EXPECT_EQ(inverse, test_state);
}

TEST_F(AESTests, mix_columns_test) {
	std::array<uint8_t, 16> known_input = {
		0xdb, 0x13, 0x53, 0x45,
		0xf2, 0x0a, 0x22, 0x5c,
		0x01, 0x01, 0x01, 0x01,
		0xc6, 0xc6, 0xc6, 0xc6
	};

	auto mixed = aes.mix_columns(known_input);
	auto inverse = aes.inv_mix_columns(mixed);
	EXPECT_EQ(inverse, known_input);
}

TEST_F(AESTests, key_expansion_test) {
	auto expanded = aes.key_expansion();

	EXPECT_EQ(expanded.size(), 4 * AES256::Nb * (AES256::Nr + 1));

	for(size_t i = 0; i < 32; i++) {
		EXPECT_EQ(expanded[i], test_key[i]);
	}
}

TEST_F(AESTests, rot_word_test) {
	auto rotated = aes.rot_word(test_word);
	EXPECT_EQ(rotated[0], test_word[1]);
	EXPECT_EQ(rotated[1], test_word[2]);
	EXPECT_EQ(rotated[2], test_word[3]);
	EXPECT_EQ(rotated[3], test_word[0]);
}

TEST_F(AESTests, sub_word_test) {
	auto subbed = aes.sub_word(test_word);
	for(size_t i = 0; i < 4; i++) {
		EXPECT_EQ(subbed[i], AES256::sbox[test_word[i]]);
	}
}

TEST_F(AESTests, add_round_key_test) {
	auto expanded_key = aes.key_expansion();
	auto result = aes.add_round_key(test_state, expanded_key, 0);

	auto reversed = aes.add_round_key(result, expanded_key, 0);
	EXPECT_EQ(reversed, test_state);
}

TEST_F(AESTests, pkcs7_padding_test) {
	std::string input = "padding";
	auto padded = aes.pkcs7_pad(input);
	int len = AES256::block_size;
	   
	EXPECT_EQ(padded.size(), len);

	uint8_t padding_value = padded.back();
	EXPECT_EQ(padding_value, len - input.length());

	for(size_t i = input.length(); i < padded.size(); i++) {
		EXPECT_EQ(padded[i], padding_value);
	}

	std::string unpadded = aes.pkcs7_unpad(padded);
	EXPECT_EQ(unpadded, input);
}
TEST_F(AESTests, invalid_pkcs7_padding_test) {
	std::vector<uint8_t> invalid_padding = {'T', 'e', 's', 't', 0x05, 0x05, 0x05, 0x04};
	EXPECT_THROW(aes.pkcs7_unpad(invalid_padding), std::invalid_argument);
}

TEST_F(AESTests, round_constants_test) {
	EXPECT_EQ(AES256::round_const[0], 0x01000000);
	EXPECT_EQ(AES256::round_const[1], 0x02000000);
	EXPECT_EQ(AES256::round_const[2], 0x04000000);
	EXPECT_EQ(AES256::round_const[3], 0x08000000);
	EXPECT_EQ(AES256::round_const[4], 0x10000000);
	EXPECT_EQ(AES256::round_const[5], 0x20000000);
	EXPECT_EQ(AES256::round_const[6], 0x40000000);
	EXPECT_EQ(AES256::round_const[7], 0x80000000);
	EXPECT_EQ(AES256::round_const[8], 0x1b000000);
	EXPECT_EQ(AES256::round_const[9], 0x36000000);
}

TEST_F(AESTests, sbox_correctnes_test) {
	const std::array<uint8_t, 256> known_sbox = {
		0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
		0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
		0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
		0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
		0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
		0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
		0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
		0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
		0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
		0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
		0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
		0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
		0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
		0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
		0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
		0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
	};

	const std::array<uint8_t, 256> known_inv_sbox = {
		0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
		0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
		0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
		0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
		0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
		0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
		0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
		0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
		0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
		0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
		0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
		0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
		0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
		0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
		0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
		0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
	};

	for (size_t i = 0; i < 256; ++i) {
		EXPECT_EQ(AES256::sbox[i], known_sbox[i])
		    << "sbox mismatch at index " << i;
	}

	for (size_t i = 0; i < 256; ++i) {
		EXPECT_EQ(AES256::inv_sbox[i], known_inv_sbox[i])
		    << "inv_sbox mismatch at index " << i;
	}
}

TEST_F(AESTests, sbox_properties_test) {
	for(size_t i = 0; i < 256; i++) {
		uint8_t original = static_cast<uint8_t>(i);
		uint8_t transformed = AES256::sbox[original];
		uint8_t inverse = AES256::inv_sbox[transformed];
		EXPECT_EQ(inverse, original);
	}
}

TEST_F(AESTests, basic_encryption_decryption_test) {
	std::string input = "primeagen4206988";
	std::string encrypted = aes.aes256_encrypt(input);
	std::string decrypted = aes.aes256_decrypt(encrypted);
	EXPECT_EQ(input, decrypted);
}

TEST_F(AESTests, empty_input_test) {
	EXPECT_THROW(aes.aes256_encrypt(""), std::invalid_argument);
	EXPECT_THROW(aes.aes256_decrypt(""), std::invalid_argument);
}

TEST_F(AESTests, block_size_alignments_test) {
	std::vector<size_t> test_lengths = {1, 15, 16, 17, 31, 32, 33};

	for (size_t len : test_lengths) {
		std::string input = generate_random_string(len);
		std::string encrypted = aes.aes256_encrypt(input);

		EXPECT_EQ(encrypted.length() % 16, 0);

		std::string decrypted = aes.aes256_decrypt(encrypted);
		EXPECT_EQ(input, decrypted);
	}
}

TEST_F(AESTests, pkcs7_padding_correctness_test) {
	std::string input = "primeagen";
	std::string encrypted = aes.aes256_encrypt(input);

	EXPECT_EQ(encrypted.length(), AES256::block_size);

	std::string decrypted = aes.aes256_decrypt(encrypted);
	EXPECT_EQ(input, decrypted);
}

TEST_F(AESTests, different_keys_produce_different_results_test) {
	std::string input = "primeagen42069";
	auto key1 = generate_aes_key<256>();
	aes.set_key(key1);
	std::string encrypted1 = aes.aes256_encrypt(input);

	auto key2 = generate_aes_key<256>();
	aes.set_key(key2);
	std::string encrypted2 = aes.aes256_encrypt(input);

	EXPECT_NE(encrypted1, encrypted2);
}

TEST_F(AESTests, large_input_handling_test) {
	std::string large_input = generate_random_string(1000);
	std::string encrypted = aes.aes256_encrypt(large_input);
	std::string decrypted = aes.aes256_decrypt(encrypted);
	EXPECT_EQ(large_input, decrypted);
}

TEST_F(AESTests, invalid_decryption_input_test) {
	std::string invalid_input = "notprimeagen";
	EXPECT_THROW(aes.aes256_decrypt(invalid_input),
	             std::invalid_argument);
}

TEST_F(AESTests, encryption_consistency_test) {
	std::string input = "primeagen42069";
	std::string encrypted1 = aes.aes256_encrypt(input);
	std::string encrypted2 = aes.aes256_encrypt(input);
	EXPECT_EQ(encrypted1, encrypted2);
}

TEST_F(AESTests, special_characters_handling_test) {
	std::string input = "!@#$%^&*()_+{}:\"|<>?~`-=[]\\;',./";
	std::string encrypted = aes.aes256_encrypt(input);
	std::string decrypted = aes.aes256_decrypt(encrypted);
	EXPECT_EQ(input, decrypted);
}

TEST_F(AESTests, unicode_characters_handling_test) {
	std::string input = "Привіт 世界 🌍";
	std::string encrypted = aes.aes256_encrypt(input);
	std::string decrypted = aes.aes256_decrypt(encrypted);
	EXPECT_EQ(input, decrypted);
}

TEST_F(AESTests, key_generation_test) {
	auto key1 = generate_aes_key<256>();
	auto key2 = generate_aes_key<256>();
	EXPECT_NE(key1, key2);
	EXPECT_EQ(key1.size(), 32);
	EXPECT_EQ(key2.size(), 32);
}

TEST_F(AESTests, tampering_detection_test) {
	std::string input = "primeagen42069";
	std::string encrypted = aes.aes256_encrypt(input);

	encrypted[0] ^= 0x01;

	std::string decrypted;
	EXPECT_NE(decrypted, input);
}

} // namespace common::crypto