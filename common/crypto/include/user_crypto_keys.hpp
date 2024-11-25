#pragma once

#include "debug.hpp"
#include "crypto_utils.hpp"

#include <boost/multiprecision/cpp_int.hpp>
#include <map>

namespace common {
namespace crypto {

using cpp_int = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>, boost::multiprecision::et_off>;

class UserCryptoKeys {
public:
    UserCryptoKeys(); 
    UserCryptoKeys(cpp_int dsa_key, cpp_int el_gamal_key, cpp_int aes_key);    
    
    const cpp_int& get_dsa_public_key() const;
    const cpp_int& get_el_gamal_public_key() const;
    const cpp_int& get_aes_key() const;
    
    void set_dsa_public_key(cpp_int key);
    void set_el_gamal_public_key(cpp_int key);
    void set_aes_key(cpp_int key);

private:
    cpp_int _dsa_public_key;
    cpp_int _el_gamal_public_key;
    cpp_int _aes_key;
};

class UserCryptoKeySet {
public:
    void add_user_keys(int user_id, const UserCryptoKeys& keys);
    std::optional<UserCryptoKeys> get_user_keys(int user_id) const;
    bool remove_user_keys(int user_id);
    bool has_user(int user_id) const;
    size_t size() const;
    void clear();
    
private:
    std::map<int, UserCryptoKeys> _key_set;
};


} // namespace crypto
} // namespace common
