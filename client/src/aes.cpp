#include <iostream>
#include <random>
#include <string>
#include <array>
#include <cstddef>
#include <iomanip>
#include <stdexcept>

namespace aes {
    
constexpr size_t Nk = 8;            // number of 32-bit words in the key (256/32 = 8)
constexpr size_t Nb = 4;            // number of columns in state (fixed in AES)
constexpr size_t Nr = 14;           // number of rounds (14 for AES-256)
    
constexpr size_t key_length = 256;  // key length in bits
constexpr size_t block_size = 128;  // block size in bits (fixed in AES)
constexpr size_t state_size = 4 * Nb;

constexpr uint8_t sbox[256] = {
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

constexpr uint8_t inv_sbox[256] = {
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

constexpr uint32_t round_const[10] = {
    0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000,
    0x20000000, 0x40000000, 0x80000000, 0x1b000000, 0x36000000
};

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

template<size_t n>
std::array<uint8_t, n / 8> generate_key() {
    static_assert(n == 128 || n == 192 || n == 256, "Key size must be 128, 192, or 256 bits");
                  
    std::array<uint8_t, n / 8> key;
    std::random_device rd;
    
    for(size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<uint8_t>(rd());
    }
    
    return key;
}

template<size_t n>
void print_key_hex(const std::array<uint8_t, n>& key) {
    std::cout << "(" << (n * 8) << " bits) hex: \n";
    for(auto byte : key) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;
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

}

int main() {
    std::string input = "DiSinGenu0uSness";
    std::string short_input = "DiSinGenu0";
    
    auto key256 = aes::generate_key<aes::key_length>();
    std::cout << "generated key: " << std::endl; 
    aes::print_key_hex(key256);
    
    std::string encrypted = aes::aes256_encrypt(input, key256);
        
    std::cout << "\nencrypted text: " << encrypted << std::endl;
    std::cout << "\nencrypted (hex): ";
    for (unsigned char c : encrypted) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                    << static_cast<int>(c) << " ";
    }
    std::cout << std::dec << std::endl;
    
    std::string decrypted = aes::aes256_decrypt(encrypted, key256);
    std::cout << "\nDecrypted: " << decrypted << std::endl;
    
    return 0;
}