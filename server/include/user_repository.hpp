#pragma once

#include "base_repository.hpp"
#include "user.hpp"
#include "db_manager.hpp"
#include <optional>

class UserRepository : public BaseRepository<User> {
public:
    UserRepository(DBManager& db_manager, const std::string& connection_name);
    virtual ~UserRepository() override;

    bool create(const User& user) override;
    std::optional<User> read(int id) override;
    bool update(const User& user) override;
    bool remove(int id) override;

    std::optional<User> findByNickname(const std::string& nickname);

private:
    User constructUser(const pqxx::row& row);
    // move inside base repo
    std::string connection_name;
};