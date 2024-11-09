#include <iostream>
#include <random>
#include <string>
#include <array>
#include <cstddef>
#include <iomanip>
#include <stdexcept>

namespace {
    
constexpr size_t Nk = 8;            // number of 32-bit words in the key (256/32 = 8)
constexpr size_t Nb = 4;            // number of columns in state (fixed in AES)
constexpr size_t Nr = 14;           // number of rounds (14 for AES-256)
    
constexpr size_t key_length = 256;  // key length in bits
constexpr size_t block_size = 128;  // block size in bits (fixed in AES)

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

constexpr uint32_t round_const[10] = {
    0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000,
    0x20000000, 0x40000000, 0x80000000, 0x1b000000, 0x36000000
};
}

std::string aes256(const std::string& input, const std::array<uint8_t, 32>& key) {
    // TODO: Implement AES-256
    return ""; 
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
    std::array<uint8_t, 4 * Nb * (Nr + 1)> expanded_keys{};
    std::array<uint8_t, 4> temp;
    
    for(size_t i = 0; i < 4 * Nk; i++) {
        expanded_keys[i] = key[i];
    }
    
    for(size_t i = Nk; i < Nb * (Nr + 1); i++) {
        for(size_t j = 0; j < 4; j++) {
            temp[j] = expanded_keys[4 * (i - 1) + j];
        }
        
        if(i % Nk == 0) {
            temp = sub_word(rot_word(temp));
            temp[0] ^= (round_const[i / Nk - 1] >> 24) & 0xFF;
        }
        else if(Nk > 6 && i % Nk == 4) {
            temp = sub_word(temp);
        }
        
        for(size_t j = 0; j < 4; j++) {
            expanded_keys[4 * i + j] = expanded_keys[4*(i-Nk) + j] ^ temp[j];
        }
    }
    
    return expanded_keys;
}


int main() {
    // we suggest that if input is bigger, then we split it and call the aes several times.
    std::string input = "DiSinGenu0uSness";
    std::string short_input = "DiSinGenu0uS";
    
    auto key256 = generate_key<key_length>();
    std::cout << "generated key: " << std::endl; 
    print_key_hex(key256);
    
    auto expanded_keys = key_expansion(key256);
    std::cout << "\nexpanded keys, there are " << expanded_keys.size() / 4 << " words :";
    for(size_t i = 0; i < expanded_keys.size(); i++) {
        if(i % 16 == 0) {
            std::cout << "\nRound " << (i / 16) << ": ";
        } 

        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(expanded_keys[i]) << " ";
    }
    
    std::cout << std::dec << std::endl;

    std::string output = aes256(input, key256);
    
    return 0;
}