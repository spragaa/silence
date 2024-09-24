#pragma once 

#include "db_manager.hpp"
#include <optional>

template<typename T>
class BaseRepository {
public:
    BaseRepository(DBManager& db_manager) : db_manager(db_manager) {}
    BaseRepository() = default;
    virtual ~BaseRepository() = default;
    
    virtual bool create(const T& entity) = 0;
    virtual std::optional<T> read(int id) = 0;
    virtual bool update(const T& entity) = 0;
    virtual bool remove(int id) = 0;
    
protected: 
    DBManager& db_manager;
};