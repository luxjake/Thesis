#pragma once

#include <cstdint>

#include "database.hpp"

class SharedDatasetInfo {

private:

    Database* identifier_database;
    Database* triples_database;
    uint64_t current_id;
    uint64_t ntriples;

    uint64_t get_new_identifier(std::string hash);

public:

    uint64_t get_identifier(std::string hash);
    uint64_t get_number_triples();

    // Add a triple to the datadb
    // Return true on success, false on failure
    bool add_triple(std::string id, std::string value);

    SharedDatasetInfo(Database* iddb);
    SharedDatasetInfo(Database* iddb, Database* datadb);

};
