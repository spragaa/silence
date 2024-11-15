#include "crypto_utils.hpp"

#include <sstream>
#include <iomanip>

namespace common {
namespace crypto {

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

cpp_int find_valid_generator(const cpp_int& p) {
    std::vector<cpp_int> candidates = { 2, 3, 5, 7, 11, 
                                        13, 17, 19, 23, 29, 
                                        31, 37, 41, 43, 47, 
                                        53, 59, 61, 67, 71, 
                                        73, 79, 83, 89, 97 };
    
    cpp_int p_1 = p - 1;
    cpp_int q = p_1 / 2;
    
    for (const auto& g : candidates) {
        if (g >= p) break;
        
        cpp_int g2 = boost::multiprecision::powm(g, cpp_int(2), p);
        cpp_int gq = boost::multiprecision::powm(g, q, p);
        
        if (g2 != 1 && gq != 1) {
            return g;
        }
    }
    
    throw std::runtime_error("No valid generator found");
}

} // namespace common
} // namespace crypto