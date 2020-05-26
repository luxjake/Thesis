#pragma once

#include <fstream>

#include "diff_result.hpp"

// Basic results between two files
class BasicDiffResult : public DiffResult {

    public:
        BasicDiffResult(std::string res_file);
        BasicDiffResult(std::string res_file, std::string add_file, std::string del_file);
        ~BasicDiffResult();

        void write_results();
        void write_results_machine();
        void print_results();

        void increment_additions();
        void increment_deletions();

        void set_nlines_file1(uint64_t n);
        void set_nlines_file2(uint64_t n);

        void set_union_size(const uint64_t size);

        void write(std::string triple, std::string file);
        void write_adddel(std::string triple, std::string ad);

        void set_result_file(const std::string f);

    private:
        uint64_t additions;
        uint64_t deletions;

        uint64_t nlines_file1;
        uint64_t nlines_file2;

        uint64_t union_size;

        std::string add_file_name;
        std::string del_file_name;
        std::ofstream add_file;
        std::ofstream del_file;

};