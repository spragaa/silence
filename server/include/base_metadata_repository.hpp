#pragma once

#include "postgres_db_manager.hpp"
#include <optional>

template<typename T>
class BaseRepository {
public:
	BaseRepository(PostgresDBManager& postgres_db_manager) : postgres_db_manager(postgres_db_manager) {
	}
	BaseRepository() = default;
	virtual ~BaseRepository() = default;

	virtual int create(const T& entity) = 0;
	virtual std::optional<T> read(int id) = 0;
	virtual bool update(const T& entity) = 0;
	virtual bool remove(int id) = 0;

protected:
	PostgresDBManager& postgres_db_manager;
};