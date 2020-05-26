#pragma once
#include <string>

class Database {

    public:
        class DatabaseIterator {
            public:
                virtual bool has_next() = 0;
                virtual std::pair<std::string, std::string> next() = 0;
                virtual void reset() = 0;
                virtual ~DatabaseIterator() {}
        };

        // Add a key->value record to the database.
        // Return a pair containing in first the inserted value (if insertion took place) and second true. Else return the existing value and false.
        virtual std::pair<std::string, bool> add_record(std::string key, std::string value) = 0;

        virtual void fast_add_record(std::string key, std::string value) = 0;

        // Remove a key->value record to the database.
        // Return true on success, false on failure.
        virtual bool remove_record(std::string key) = 0;
        // Check if a record identified by key exist in the database.
        // Return true on success, false on failure.
        virtual bool exist(std::string key) = 0;
        // Get the value of a record in the 'value' parameter
        // Return true on success, false on failure.
        virtual bool get_record(std::string key, std::string* value) = 0;

        virtual DatabaseIterator* get_iterator() = 0;

        virtual ~Database() {}

        // TODO:
        // Merge two Database
        // Does not modify the input database. It is the responsibility of the calling code to discard the dbs from memory after completion if necessary
        // Return a new Database that is the result of the merge
        static Database* merge(const Database& db1, const Database& db2) {
            return nullptr;
        }
    
};