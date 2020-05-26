#pragma once
#include <string>

class DiffResult {

    public:
        virtual void write_results() = 0;
        virtual void print_results() = 0;
        virtual void set_result_file(std::string f) = 0;
        virtual ~DiffResult() {}

    protected:
        std::string result_file;

};