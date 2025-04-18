#pragma once

#include "base_metadata_repository.hpp"
#include "user.hpp"
#include "postgres_db_manager.hpp"
#include "user_crypto_keys.hpp"

#include <optional>
#include <nlohmann/json.hpp>

namespace server {

class UserMetadataRepository : public BaseRepository<common::User> {
public:
	UserMetadataRepository(PostgresDBManager& postgres_db_manager, const std::string& connection_name);
	virtual ~UserMetadataRepository() override;

	int create(const common::User& user) override;
	std::optional<common::User> read(int id) override;
	bool update(const common::User& user) override;
	bool remove(int id) override;

	bool authorize(int user_id, const std::string& nickname, const std::string& password);
	int get_id(const std::string& nickname);
	bool set_public_keys(const int user_id, const std::string& el_gamal_public_key, const std::string& dsa_public_key);
	std::optional<crypto::UserCryptoKeys> get_public_keys(const int user_id);

private:
	common::User construct_user(const nlohmann::json& user_json);
	crypto::UserCryptoKeys construct_user_crypto_keys(const nlohmann::json& user_crypto_keys_json);
	nlohmann::json pqxx_users_result_to_json(const pqxx::result& r) const;
	nlohmann::json pqxx_crypto_keys_result_to_json(const pqxx::result& r) const;


private:
	std::string _connection_name; // move inside base repo
};

}