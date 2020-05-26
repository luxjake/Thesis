#include <kcpolydb.h>
#include "database.hpp"

class MultiKCDatabase : public Database {

    public:
        class MultiKCIterator : public Database::DatabaseIterator {
            public:
                //MultiKCIterator(MultiKCDatabase* db, kyotocabinet::PolyDB::Cursor* cursor);
                MultiKCIterator(MultiKCDatabase* db, std::vector<kyotocabinet::PolyDB::Cursor*>* cursors);
                void reset();
                bool has_next();
                std::pair<std::string, std::string> next();  

            protected:
                MultiKCDatabase* db;
                std::vector<kyotocabinet::PolyDB::Cursor*>* cursors;
                std::string current_key;
                std::string current_value;
                bool hn;
                unsigned current_db;

        };

        MultiKCDatabase(std::string path);
        ~MultiKCDatabase();

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

        MultiKCIterator* get_iterator();

    protected:
        MultiKCIterator* it;

    private:
        const unsigned ndb;
        std::vector<kyotocabinet::PolyDB*> db;
        std::vector<kyotocabinet::PolyDB::Cursor*> cursors;

};