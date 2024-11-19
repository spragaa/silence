#include "aes256.hpp"

namespace common {
namespace crypto {

constexpr size_t AES256::Nk;
constexpr size_t AES256::Nb;
constexpr size_t AES256::Nr;
constexpr size_t AES256::key_length;
constexpr size_t AES256::block_size;
constexpr size_t AES256::state_size;
    
constexpr uint8_t AES256::sbox[256];
    
constexpr uint8_t AES256::inv_sbox[256];
    
constexpr uint32_t AES256::round_const[10];
    
AES256::AES256() {
    _key = generate_aes_key<256>();
    // log
}    

AES256::AES256(const std::array<uint8_t, AES256::key_length/8>& key) : _key(key) {
    // log
}
 
void AES256::set_key(const std::array<uint8_t, key_length/8> key) {
    _key = key;
    // log?
}

uint8_t AES256::gmul(uint8_t a, uint8_t b) {
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

std::array<uint8_t, 4> AES256::rot_word(const std::array<uint8_t, 4>& word) {
	std::array<uint8_t, 4> output;

	output[0] = word[1];
	output[1] = word[2];
	output[2] = word[3];
	output[3] = word[0];

	return output;
}

std::array<uint8_t, 4> AES256::sub_word(const std::array<uint8_t, 4>& word) {
	std::array<uint8_t, 4> output;

	output[0] = AES256::sbox[word[0]];
	output[1] = AES256::sbox[word[1]];
	output[2] = AES256::sbox[word[2]];
	output[3] = AES256::sbox[word[3]];

	return output;
}

// std::array<uint8_t, 4 * AES256::Nb * (AES256::Nr + 1)> AES256::key_expansion(const std::array<uint8_t, 4 * AES256::Nk>& _key) {
std::array<uint8_t, 4 * AES256::Nb * (AES256::Nr + 1)> AES256::key_expansion() {
	std::array<uint8_t, 4 * AES256::Nb * (AES256::Nr + 1)> w{};
	std::array<uint8_t, 4> temp;

	for(size_t i = 0; i < 4 * AES256::Nk; i++) {
		w[i] = _key[i];
	}

	for(size_t i = AES256::Nk; i < AES256::Nb * (AES256::Nr + 1); i++) {
		for(size_t j = 0; j < 4; j++) {
			temp[j] = w[4 * (i - 1) + j];
		}

		if(i % AES256::Nk == 0) {
			temp = sub_word(rot_word(temp));
			temp[0] ^= (AES256::round_const[i / AES256::Nk - 1] >> 24) & 0xFF;
		}
		else if(AES256::Nk > 6 && i % AES256::Nk == 4) {
			temp = sub_word(temp);
		}

		for(size_t j = 0; j < 4; j++) {
			w[4 * i + j] = w[4*(i-AES256::Nk) + j] ^ temp[j];
		}
	}

	DEBUG_MSG("[aes256::key_expansion] Expanded _key: " + bytes_array_to_string(w));

	return w;
}

std::array<uint8_t, AES256::state_size> AES256::sub_bytes(const std::array<uint8_t, AES256::state_size>& state) {
	std::array<uint8_t, AES256::state_size> output;

	for (size_t i = 0; i < AES256::state_size; i++) {
		output[i] = AES256::sbox[state[i]];
	}

	return output;
}

std::array<uint8_t, AES256::state_size> AES256::inv_sub_bytes(const std::array<uint8_t, AES256::state_size>& state) {
	std::array<uint8_t, AES256::state_size> output;

	for (size_t i = 0; i < AES256::state_size; i++) {
		output[i] = AES256::inv_sbox[state[i]];
	}

	return output;
}

std::array<uint8_t, AES256::state_size> AES256::shift_rows(const std::array<uint8_t, AES256::state_size>& state) {
	std::array<uint8_t, AES256::state_size> output;

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

std::array<uint8_t, AES256::state_size> AES256::inv_shift_rows(const std::array<uint8_t, AES256::state_size>& state) {
	std::array<uint8_t, AES256::state_size> output;

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

std::array<uint8_t, AES256::state_size> AES256::mix_columns(const std::array<uint8_t, AES256::state_size>& state) {
	std::array<uint8_t, AES256::state_size> output;

	for (int c = 0; c < 4; c++) {
		uint8_t s0 = state[4 * c];
		uint8_t s1 = state[4 * c + 1];
		uint8_t s2 = state[4 * c + 2];
		uint8_t s3 = state[4 * c + 3];

		output[4 * c] = gmul(0x02, s0) ^ gmul(0x03, s1) ^ s2 ^ s3;
		output[4 * c + 1] = s0 ^ gmul(0x02, s1) ^ gmul(0x03, s2) ^ s3;
		output[4 * c + 2] = s0 ^ s1 ^ gmul(0x02, s2) ^ gmul(0x03, s3);
		output[4 * c + 3] = gmul(0x03, s0) ^ s1 ^ s2 ^ gmul(0x02, s3);
	}

	return output;
}

std::array<uint8_t, AES256::state_size> AES256::inv_mix_columns(const std::array<uint8_t, AES256::state_size>& state) {
	std::array<uint8_t, AES256::state_size> output;

	for (int c = 0; c < 4; c++) {
		uint8_t s0 = state[4 * c];
		uint8_t s1 = state[4 * c + 1];
		uint8_t s2 = state[4 * c + 2];
		uint8_t s3 = state[4 * c + 3];

		output[4 * c] = gmul(0x0e, s0) ^ gmul(0x0b, s1) ^ gmul(0x0d, s2) ^ gmul(0x09, s3);
		output[4 * c + 1] = gmul(0x09, s0) ^ gmul(0x0e, s1) ^ gmul(0x0b, s2) ^ gmul(0x0d, s3);
		output[4 * c + 2] = gmul(0x0d, s0) ^ gmul(0x09, s1) ^ gmul(0x0e, s2) ^ gmul(0x0b, s3);
		output[4 * c + 3] = gmul(0x0b, s0) ^ gmul(0x0d, s1) ^ gmul(0x09, s2) ^ gmul(0x0e, s3);
	}

	return output;
}

std::array<uint8_t, AES256::state_size> AES256::add_round_key(
	const std::array<uint8_t, AES256::state_size>& state,
	const std::array<uint8_t, 4 * AES256::Nb * (AES256::Nr + 1)>& w,
	int round) {

	std::array<uint8_t, AES256::state_size> output;

	for (int c = 0; c < 4; c++) {
		for (int r = 0; r < 4; r++) {
			output[r + 4 * c] = state[r + 4 * c] ^ w[4 * round * 4 + 4 * c + r];
		}
	}

	return output;
}

std::vector<uint8_t> AES256::pkcs7_pad(const std::string& input) {
	size_t padding_len = AES256::block_size - (input.length() % AES256::block_size);
	if (padding_len == 0) {
		padding_len = AES256::block_size;
	}

	std::vector<uint8_t> padded(input.begin(), input.end());
	for (size_t i = 0; i < padding_len; i++) {
		padded.push_back(static_cast<uint8_t>(padding_len));
	}

	return padded;
}

std::string AES256::pkcs7_unpad(const std::vector<uint8_t>& input) {
	if (input.empty()) {
		FATAL_MSG("[aes256::pkcs7_unpad] Input cannot be empty");
		throw std::invalid_argument("Input cannot be empty");
	}

	uint8_t padding_len = input.back();
	if (padding_len == 0 || padding_len > input.size()) {
		FATAL_MSG("[aes256::pkcs7_unpad] Invalid padding");
		throw std::invalid_argument("Invalid padding");
	}

	for (size_t i = input.size() - padding_len; i < input.size(); i++) {
		if (input[i] != padding_len) {
			FATAL_MSG("[aes256::pkcs7_unpad] Invalid padding");
			throw std::invalid_argument("Invalid padding");
		}
	}

	std::string unpadded = std::string(input.begin(), input.end() - padding_len);
	DEBUG_MSG("[aes256::pkcs7_unpad] Unpaded result: " + unpadded);
	return unpadded;
}

std::array<uint8_t, AES256::state_size> AES256::encrypt(const std::array<uint8_t, AES256::state_size>& input) {

	std::array<uint8_t, AES256::state_size> state = input;

	auto w = key_expansion();

	state = add_round_key(state, w, 0);

	for (int round = 1; round < AES256::Nr; round++) {
		state = sub_bytes(state);
		state = shift_rows(state);
		state = mix_columns(state);
		state = add_round_key(state, w, round);

		// add another variance of debug msg?
		// DEBUG_MSG("[aes256::encrypt] State after round " + std::to_string(round) + ":" + bytes_array_to_string(state));
	}

	state = sub_bytes(state);
	state = shift_rows(state);
	state = add_round_key(state, w, AES256::Nr);
	DEBUG_MSG("[aes256::encrypt] State after final round: " + bytes_array_to_string(state));

	return state;
}

std::string AES256::aes256_encrypt(const std::string& input) {
	if (input.empty()) {
		FATAL_MSG("[aes256::aes256_encrypt] Input cannot be empty");
		throw std::invalid_argument("Input cannot be empty");
	}

	auto padded = pkcs7_pad(input);

	std::string output;
	for (size_t i = 0; i < padded.size(); i += AES256::state_size) {
		std::array<uint8_t, AES256::state_size> block{};
		std::copy(padded.begin() + i, padded.begin() + i + AES256::state_size, block.begin());

		auto encrypted_block = encrypt(block);
		output.append(encrypted_block.begin(), encrypted_block.end());
	}

	return output;
}
std::array<uint8_t, AES256::state_size> AES256::decrypt(const std::array<uint8_t, AES256::state_size>& input) {

	std::array<uint8_t, AES256::state_size> state = input;

	auto w = key_expansion();

	state = add_round_key(state, w, AES256::Nr);

	for (int round = AES256::Nr - 1; round >= 1; round--) {
		state = inv_shift_rows(state);
		state = inv_sub_bytes(state);
		state = add_round_key(state, w, round);
		state = inv_mix_columns(state);

		// DEBUG_MSG("[aes256::decrypt] State after round " + std::to_string(round) + ":" + bytes_array_to_string(state));
	}

	state = inv_shift_rows(state);
	state = inv_sub_bytes(state);
	state = add_round_key(state, w, 0);

	// add another variance of debug msg?
	DEBUG_MSG("[aes256::decrypt] State after final round: " + bytes_array_to_string(state));

	return state;
}

std::string AES256::aes256_decrypt(const std::string& input) {
	if (input.empty() || input.length() % AES256::state_size != 0) {
		FATAL_MSG("[aes256::aes256_decrypt] Input length must be multiple of block size");
		throw std::invalid_argument("Input length must be multiple of block size");
	}

	std::vector<uint8_t> decrypted;
	for (size_t i = 0; i < input.length(); i += AES256::state_size) {
		std::array<uint8_t, AES256::state_size> block;
		std::copy(input.begin() + i, input.begin() + i + AES256::state_size, block.begin());

		auto decrypted_block = decrypt(block);
		decrypted.insert(decrypted.end(), decrypted_block.begin(), decrypted_block.end());
	}

	return pkcs7_unpad(decrypted);
}

} // namespace crypto
} // namespace common
