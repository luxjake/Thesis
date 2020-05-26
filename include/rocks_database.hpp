#pragma once

#include "rocksdb/db.h"
#include "database.hpp"

class RocksDatabase : public Database {

    public:        
        class RocksDbIterator : public Database::DatabaseIterator {
            public:
                RocksDbIterator(rocksdb::DB* db);
                void reset();
                bool has_next();
                std::pair<std::string, std::string> next();

            private:
                rocksdb::DB* db;
                rocksdb::Iterator* it;
        };

        RocksDatabase(std::string path);
        ~RocksDatabase();

        // Add a key->value record to the database.
        // Return a pair containing in first the inserted value (if insertion took place) and second true. Else return the existing value and false.
        std::pair<std::string, bool> add_record(std::string key, std::string value);
        void fast_add_record(std::string key, std::string value);
        // Remove a key->value record to the database.
        // Return true on success, false on failure.
        bool remove_record(std::string key);
        // Check if a record identified by key exist in the database.
        // Return true on success, false on failure.
        bool exist(std::string key);
        // Get the value of a record in the 'value' parameter
        // Return true on success, false on failure.
        bool get_record(std::string key, std::string* value);

        RocksDbIterator* get_iterator();

    protected:
        RocksDbIterator* it;
        rocksdb::DB* db;

};