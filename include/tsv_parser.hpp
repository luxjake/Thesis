#pragma once
#include <parser.hpp>

#include "shared_dataset_info.hpp"

class TSVParser : public Parser {

    public:
        TSVParser(std::vector<bool>* bitvec, SharedDatasetInfo* sdi, bool store_triple);
        TSVParser(std::vector<bool>* bitvec, SharedDatasetInfo* sdi, bool store_triple, uint64_t id);
        void read_file(std::string file);
        u_int64_t get_triple_id();
        u_int64_t get_number_of_triples();
        
        ~TSVParser();

};