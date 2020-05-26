#include <kchashdb.h>
#include "database.hpp"

class KCDatabase : public Database {

    public:
        class KCIterator : public Database::DatabaseIterator {
            public:
                KCIterator(kyotocabinet::HashDB::Cursor* cursor);
                void reset();
                bool has_next();
                std::pair<std::string, std::string> next();  

            protected:
                kyotocabinet::HashDB::Cursor* cursor;
                bool hn;
                std::string current_key;
                std::string current_value;

        };

        KCDatabase(std::string path);
        ~KCDatabase();

        // Add a key->value record to the database.
        // Return true on success, false on failure.
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

        KCIterator* get_iterator();

    protected:
        KCIterator* it;

    private:
        kyotocabinet::HashDB* db;
        kyotocabinet::Compressor* comp;

};



