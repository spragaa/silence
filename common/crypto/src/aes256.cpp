#include "aes256.hpp"

namespace common {
namespace crypto {

uint8_t gmul(uint8_t a, uint8_t b) {
	uint8_t p = 0;
	uint8_t hi_bit_set;

	for (int i = 0; i < 8; i++) {
		if (b & 1) {
			p ^= a;
		}

		hi_bit_set = (a & 0x80);
		a <<= 1;

		if (hi_bit_set) {
			a ^= 0x1b; // x^8 + x^4 + x^3 + x + 1
		}

		b >>= 1;
	}
	return p;
}

std::array<uint8_t, 4> rot_word(const std::array<uint8_t, 4>& word) {
	std::array<uint8_t, 4> output;

	output[0] = word[1];
	output[1] = word[2];
	output[2] = word[3];
	output[3] = word[0];

	return output;
}

std::array<uint8_t, 4> sub_word(const std::array<uint8_t, 4>& word) {
	std::array<uint8_t, 4> output;

	output[0] = sbox[word[0]];
	output[1] = sbox[word[1]];
	output[2] = sbox[word[2]];
	output[3] = sbox[word[3]];

	return output;
}

std::array<uint8_t, 4 * Nb * (Nr + 1)> key_expansion(const std::array<uint8_t, 4 * Nk>& key) {
	std::array<uint8_t, 4 * Nb * (Nr + 1)> w{};
	std::array<uint8_t, 4> temp;

	for(size_t i = 0; i < 4 * Nk; i++) {
		w[i] = key[i];
	}

	for(size_t i = Nk; i < Nb * (Nr + 1); i++) {
		for(size_t j = 0; j < 4; j++) {
			temp[j] = w[4 * (i - 1) + j];
		}

		if(i % Nk == 0) {
			temp = sub_word(rot_word(temp));
			temp[0] ^= (round_const[i / Nk - 1] >> 24) & 0xFF;
		}
		else if(Nk > 6 && i % Nk == 4) {
			temp = sub_word(temp);
		}

		for(size_t j = 0; j < 4; j++) {
			w[4 * i + j] = w[4*(i-Nk) + j] ^ temp[j];
		}
	}

	return w;
}

std::array<uint8_t, state_size> sub_bytes(const std::array<uint8_t, state_size>& state) {
	std::array<uint8_t, state_size> output;

	for (size_t i = 0; i < state_size; i++) {
		output[i] = sbox[state[i]];
	}

	return output;
}

std::array<uint8_t, state_size> inv_sub_bytes(const std::array<uint8_t, state_size>& state) {
	std::array<uint8_t, state_size> output;

	for (size_t i = 0; i < state_size; i++) {
		output[i] = inv_sbox[state[i]];
	}

	return output;
}

std::array<uint8_t, state_size> shift_rows(const std::array<uint8_t, state_size>& state) {
	std::array<uint8_t, state_size> output;

	// for row i shift it by i pos to the left

	output[0] = state[0];
	output[4] = state[4];
	output[8] = state[8];
	output[12] = state[12];

	output[1] = state[5];
	output[5] = state[9];
	output[9] = state[13];
	output[13] = state[1];

	output[2] = state[10];
	output[6] = state[14];
	output[10] = state[2];
	output[14] = state[6];

	output[3] = state[15];
	output[7] = state[3];
	output[11] = state[7];
	output[15] = state[11];

	return output;
}

std::array<uint8_t, state_size> inv_shift_rows(const std::array<uint8_t, state_size>& state) {
	std::array<uint8_t, state_size> output;

	// for row i shift right by i

	output[0] = state[0];
	output[4] = state[4];
	output[8] = state[8];
	output[12] = state[12];

	output[1] = state[13];
	output[5] = state[1];
	output[9] = state[5];
	output[13] = state[9];

	output[2] = state[10];
	output[6] = state[14];
	output[10] = state[2];
	output[14] = state[6];

	output[3] = state[7];
	output[7] = state[11];
	output[11] = state[15];
	output[15] = state[3];

	return output;
}

std::array<uint8_t, state_size> mix_columns(const std::array<uint8_t, state_size>& state) {
	std::array<uint8_t, state_size> output;

	for (int c = 0; c < 4; c++) {
		uint8_t s0 = state[4*c];
		uint8_t s1 = state[4*c + 1];
		uint8_t s2 = state[4*c + 2];
		uint8_t s3 = state[4*c + 3];

		output[4*c] = gmul(0x02, s0) ^ gmul(0x03, s1) ^ s2 ^ s3;
		output[4*c + 1] = s0 ^ gmul(0x02, s1) ^ gmul(0x03, s2) ^ s3;
		output[4*c + 2] = s0 ^ s1 ^ gmul(0x02, s2) ^ gmul(0x03, s3);
		output[4*c + 3] = gmul(0x03, s0) ^ s1 ^ s2 ^ gmul(0x02, s3);
	}

	return output;
}

std::array<uint8_t, state_size> inv_mix_columns(const std::array<uint8_t, state_size>& state) {
	std::array<uint8_t, state_size> output;

	for (int c = 0; c < 4; c++) {
		uint8_t s0 = state[4*c];
		uint8_t s1 = state[4*c + 1];
		uint8_t s2 = state[4*c + 2];
		uint8_t s3 = state[4*c + 3];

		output[4*c] = gmul(0x0e, s0) ^ gmul(0x0b, s1) ^ gmul(0x0d, s2) ^ gmul(0x09, s3);
		output[4*c + 1] = gmul(0x09, s0) ^ gmul(0x0e, s1) ^ gmul(0x0b, s2) ^ gmul(0x0d, s3);
		output[4*c + 2] = gmul(0x0d, s0) ^ gmul(0x09, s1) ^ gmul(0x0e, s2) ^ gmul(0x0b, s3);
		output[4*c + 3] = gmul(0x0b, s0) ^ gmul(0x0d, s1) ^ gmul(0x09, s2) ^ gmul(0x0e, s3);
	}

	return output;
}

std::array<uint8_t, state_size> add_round_key(
	const std::array<uint8_t, state_size>& state,
	const std::array<uint8_t, 4 * Nb * (Nr + 1)>& w,
	int round) {

	std::array<uint8_t, state_size> output;

	for (int c = 0; c < 4; c++) {
		for (int r = 0; r < 4; r++) {
			output[r + 4 * c] = state[r + 4 * c] ^ w[4 * round * 4 + 4 * c + r];
		}
	}

	return output;
}

std::vector<uint8_t> pkcs7_pad(const std::string& input, size_t block_size) {
	size_t padding_len = block_size - (input.length() % block_size);
	if (padding_len == 0) {
		padding_len = block_size;
	}

	std::vector<uint8_t> padded(input.begin(), input.end());
	for (size_t i = 0; i < padding_len; i++) {
		padded.push_back(static_cast<uint8_t>(padding_len));
	}

	return padded;
}

std::string pkcs7_unpad(const std::vector<uint8_t>& input) {
	if (input.empty()) {
		throw std::invalid_argument("Input cannot be empty");
	}

	uint8_t padding_len = input.back();
	if (padding_len == 0 || padding_len > input.size()) {
		throw std::invalid_argument("Invalid padding");
	}

	for (size_t i = input.size() - padding_len; i < input.size(); i++) {
		if (input[i] != padding_len) {
			throw std::invalid_argument("Invalid padding");
		}
	}

	return std::string(input.begin(), input.end() - padding_len);
}

std::array<uint8_t, state_size> encrypt(
	const std::array<uint8_t, state_size>& input,
	const std::array<uint8_t, 32>& key) {

	std::array<uint8_t, state_size> state = input;

	auto w = key_expansion(key);

	state = add_round_key(state, w, 0);

	for (int round = 1; round < Nr; round++) {
		state = sub_bytes(state);
		state = shift_rows(state);
		state = mix_columns(state);
		state = add_round_key(state, w, round);
	}

	state = sub_bytes(state);
	state = shift_rows(state);
	state = add_round_key(state, w, Nr);

	return state;
}

std::string aes256_encrypt(const std::string& input, const std::array<uint8_t, 32>& key) {
	if (input.empty()) {
		throw std::invalid_argument("Input cannot be empty");
	}

	auto padded = pkcs7_pad(input, state_size);

	std::string output;
	for (size_t i = 0; i < padded.size(); i += state_size) {
		std::array<uint8_t, state_size> block{};
		std::copy(padded.begin() + i, padded.begin() + i + state_size, block.begin());

		auto encrypted_block = encrypt(block, key);
		output.append(encrypted_block.begin(), encrypted_block.end());
	}

	return output;
}

std::array<uint8_t, state_size> decrypt(
	const std::array<uint8_t, state_size>& input,
	const std::array<uint8_t, 32>& key) {

	std::array<uint8_t, state_size> state = input;

	auto w = key_expansion(key);

	state = add_round_key(state, w, Nr);

	for (int round = Nr - 1; round >= 1; round--) {
		state = inv_shift_rows(state);
		state = inv_sub_bytes(state);
		state = add_round_key(state, w, round);
		state = inv_mix_columns(state);
	}

	state = inv_shift_rows(state);
	state = inv_sub_bytes(state);
	state = add_round_key(state, w, 0);

	return state;
}

std::string aes256_decrypt(const std::string& input, const std::array<uint8_t, 32>& key) {
	if (input.empty() || input.length() % state_size != 0) {
		throw std::invalid_argument("Input length must be multiple of block size");
	}

	std::vector<uint8_t> decrypted;
	for (size_t i = 0; i < input.length(); i += state_size) {
		std::array<uint8_t, state_size> block;
		std::copy(input.begin() + i, input.begin() + i + state_size, block.begin());

		auto decrypted_block = decrypt(block, key);
		decrypted.insert(decrypted.end(), decrypted_block.begin(), decrypted_block.end());
	}

	return pkcs7_unpad(decrypted);
}

} // namespace crypto
} // namespace common

// int main() {
//     std::string input = "DiSinGenu0uSness";
//     std::string short_input = "DiSinGenu0";

//     auto key256 = common::crypto::generate_key<common::crypto::key_length>();
//     std::cout << "generatesd key: " << std::endl;
//     common::crypto::print_key_hex(key256);

//     std::string encrypted = common::crypto::aes256_encrypt(input, key256);

//     std::cout << "\nencrypted text: " << encrypted << std::endl;
//     std::cout << "\nencrypted (hex): ";
//     for (unsigned char c : encrypted) {
//         std::cout << std::hex << std::setw(2) << std::setfill('0')
//                     << static_cast<int>(c) << " ";
//     }
//     std::cout << std::dec << std::endl;

//     std::string decrypted = common::crypto::aes256_decrypt(encrypted, key256);
//     std::cout << "\nDecrypted: " << decrypted << std::endl;

//     return 0;
// }