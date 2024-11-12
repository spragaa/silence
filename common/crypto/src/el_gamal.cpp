#include "el_gamal.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

cpp_int hex_to_cpp_int(const std::string& hex) {
    cpp_int result;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> result;
    return result;
}

std::string cpp_int_to_hex(const cpp_int& num) {
    std::stringstream ss;
    ss << std::hex << num;
    return ss.str();
}

int main() {
    cpp_int p("0xFFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74"
              "020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F1437"
              "4FE1356D6D51C245E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"
              "EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF05"
              "98DA48361C55D39A69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB"
              "9ED529077096966D670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B"
              "E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF695581718"
              "3995497CEA956AE515D2261898FA051015728E5A8AACAA68FFFFFFFFFFFFFFFF");

    cpp_int g("2");

    ElGamal alice(p, g);
    
    ElGamal bob(p, g);

    cpp_int aes_key = hex_to_cpp_int("0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF");

    std::cout << "Original AES Key: " << cpp_int_to_hex(aes_key) << std::endl;

    auto encrypted = alice.encrypt(aes_key, bob.get_keys().public_key);

    auto decrypted = bob.decrypt(encrypted);

    std::cout << "Decrypted AES Key: " << cpp_int_to_hex(decrypted) << std::endl;
    std::cout << "Keys match: " << (aes_key == decrypted ? "Yes" : "No") << std::endl;

    return 0;
}