#include "user_crypto_keys.hpp"

namespace common {
namespace crypto {

UserCryptoKeys::UserCryptoKeys()
	: _dsa_public_key(0)
	, _el_gamal_public_key(0)
	, _aes_key(0) {
}

UserCryptoKeys::UserCryptoKeys(cpp_int dsa_key, cpp_int el_gamal_key, cpp_int aes_key)
	: _dsa_public_key(std::move(dsa_key))
	, _el_gamal_public_key(std::move(el_gamal_key))
	, _aes_key(std::move(aes_key)) {
}

const cpp_int& UserCryptoKeys::get_dsa_public_key() const {
	return _dsa_public_key;
}

const cpp_int& UserCryptoKeys::get_el_gamal_public_key() const {
	return _el_gamal_public_key;
}

const cpp_int& UserCryptoKeys::get_aes_key() const {
	return _aes_key;
}

void UserCryptoKeys::set_dsa_public_key(cpp_int key) {
	_dsa_public_key = std::move(key);
	DEBUG_MSG("[UserCryptoKeys::set_dsa_public_key] DSA key set: " + cpp_int_to_hex(_dsa_public_key));
}

void UserCryptoKeys::set_el_gamal_public_key(cpp_int key) {
	_el_gamal_public_key = std::move(key);
	DEBUG_MSG("[UserCryptoKeys::set_el_gamal_public_key] El gamal key set: " + cpp_int_to_hex(_el_gamal_public_key));
}

void UserCryptoKeys::set_aes_key(cpp_int key) {
	_aes_key = std::move(key);
	DEBUG_MSG("[UserCryptoKeys::set_aes_key] AES key set: " + cpp_int_to_hex(_aes_key));
}

void UserCryptoKeySet::add_user_keys(int user_id, const UserCryptoKeys& keys) {
	_key_set[user_id] = keys;
	DEBUG_MSG("[UserCryptoKeys::add_user_keys] User added, with id " + std::to_string(user_id) + ", with keys:  el gamal: "
	          + cpp_int_to_hex(_key_set[user_id].get_el_gamal_public_key())
	          + "dsa: " + cpp_int_to_hex(_key_set[user_id].get_dsa_public_key())
	          + "aes: " + cpp_int_to_hex(_key_set[user_id].get_aes_key())
	          );
}

std::optional<UserCryptoKeys> UserCryptoKeySet::get_user_keys(int user_id) const {
	auto it = _key_set.find(user_id);
	if (it != _key_set.end()) {
		return it->second;
	}
	return std::nullopt;
}

bool UserCryptoKeySet::remove_user_keys(int user_id) {
	return _key_set.erase(user_id) > 0;
}

bool UserCryptoKeySet::has_user(int user_id) const {
	return _key_set.find(user_id) != _key_set.end();
}

size_t UserCryptoKeySet::size() const {
	return _key_set.size();
}

void UserCryptoKeySet::clear() {
	_key_set.clear();
}

UserCryptoKeys UserCryptoKeys::from_json(const nlohmann::json& json) {
	try {
		cpp_int dsa_key = json["dsa_public_key"].is_null() ?
		                  cpp_int(-1) : hex_to_cpp_int(json["dsa_public_key"].get<std::string>());

		cpp_int el_gamal_key = json["el_gamal_public_key"].is_null() ?
		                       cpp_int(-1) : hex_to_cpp_int(json["el_gamal_public_key"].get<std::string>());

		return UserCryptoKeys(
			dsa_key,
			el_gamal_key,
			cpp_int(-1)
			);
	} catch (const std::exception& e) {
		ERROR_MSG("[UserCryptoKeys::from_json] Error parsing JSON: " + std::string(e.what()));
		return UserCryptoKeys(cpp_int(-1), cpp_int(-1), cpp_int(-1));
	}
}

} // namespace crypto
} // namespace common
