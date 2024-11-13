#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <string>

namespace common {
namespace crypto {

using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_off>;

struct KeyPair {
	cpp_int private_key;
	cpp_int public_key;
};

struct EncryptedMessage {
	cpp_int c1;
	cpp_int c2;
};


cpp_int hex_to_cpp_int(const std::string& hex);
std::string cpp_int_to_hex(const cpp_int& num);

} // namespace common
} // namespace crypto