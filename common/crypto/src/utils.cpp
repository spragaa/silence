#include "utils.hpp"

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

} // namespace common
} // namespace crypto