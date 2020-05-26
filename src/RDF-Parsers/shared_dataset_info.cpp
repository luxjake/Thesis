#include <iostream>
#include <omp.h>

#include "shared_dataset_info.hpp"

SharedDatasetInfo::SharedDatasetInfo(Database* iddb) : current_id(0), ntriples(0) {
    identifier_database = iddb;
}

SharedDatasetInfo::SharedDatasetInfo(Database* iddb, Database* datadb) : current_id(0), ntriples(0) {
    identifier_database = iddb;
    triples_database = datadb;
}

//SharedDatasetInfo::~SharedDatasetInfo() {}

uint64_t SharedDatasetInfo::get_new_identifier(std::string hash) {
    uint64_t tid = -1;
    tid = current_id;
    auto r = identifier_database->add_record(hash, std::to_string(current_id));
    if (r.second) { // succesfully added the id to the database
        current_id++;
        ntriples++;
    } else { // id already in the db, we use the existing value
        tid = std::stoul(r.first);
    }
    return tid;
}

uint64_t SharedDatasetInfo::get_identifier(std::string hash) {
    uint64_t tid = 0;    
    std::string tmp;
    #pragma omp critical(sdi_get_id)
    {  
        if (identifier_database->get_record(hash, &tmp)) {
            tid = std::stoul(tmp);
            if (current_id < tid) {
                current_id = tid;
            }
        } else {
            tid = get_new_identifier(hash);
        }
    }
    return tid;
}

bool SharedDatasetInfo::add_triple(std::string id, std::string value) {
    bool s;
    #pragma omp critical(sdi_add_triple)
    {
        if (triples_database) {
            //auto r = triples_database->add_record(id, value);
            //s = r.second;
            triples_database->fast_add_record(id, value);
            s = true;
        } else {
            std::cerr << "No triple database exist." << std::endl;
            s = false;
        }
    }    
    return s;
}

uint64_t SharedDatasetInfo::get_number_triples() {
    return ntriples;
}