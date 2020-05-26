#pragma once

#include <unordered_map>

#include "database.hpp"

class MapDatabase : public Database {

    public:        
        class MapDbIterator : public Database::DatabaseIterator {
            public:
                MapDbIterator(std::unordered_map<size_t, std::string>* db);
                void reset();
                bool has_next();
                std::pair<std::string, std::string> next();

            protected:
                std::unordered_map<size_t, std::string>* db;
            
            private:
                std::unordered_map<size_t, std::string>::iterator it;
        };

        MapDatabase();
        ~MapDatabase();

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

        MapDbIterator* get_iterator();

    protected:
        MapDbIterator* it;

    private:
        std::unordered_map<size_t, std::string>* mapdb;

};