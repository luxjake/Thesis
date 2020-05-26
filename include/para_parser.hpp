#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

#include <parser.hpp>

#include "shared_dataset_info.hpp"

class ParallelParser : public Parser {

    private:
        bool stop;
        uint64_t processed_lines;
        std::string delimiter;
        std::queue<std::string> to_process;
        std::mutex lock;
        std::condition_variable cond_var;

        std::mutex bitvec_lock;

        void add_to_process_list(std::string line);
        std::string parse_line(std::string line);
        void parsing_task();
        void reading_task(std::string file);

    public:
        ParallelParser(std::vector<bool>* bitvec, SharedDatasetInfo* sdi, bool store_triples);
        void read_file(std::string file);
        u_int64_t get_triple_id();
        u_int64_t get_number_of_triples();
        
        ~ParallelParser();

};