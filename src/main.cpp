#include <iostream>
#include <omp.h>

#include "tsv_parser.hpp"
#include "basic_diff_result.hpp"
#include "bitsets_diff.hpp"

int main(int argc, char **argv) {

    // Need the following commands :
    // - store <rdf file>
    // - diff <rdf file 1> <rdf file 2>
    // - dirdiff <directory>

    if (argc != 5) {
        std::cerr << "3 arguments needed :\n- the two files to compare\n- the database type (onmemory/ondisk)\n- comparison name (use for the result file)" << std::endl;
        return -1;
    }

    omp_set_nested(1);
    omp_set_dynamic(1);

    std::string file1 = argv[1];
    std::string file2 = argv[2];
    std::string dbtype = argv[3];
    std::string compname = argv[4];

    std::cout << "Using " << dbtype << " database." << std::endl;

    std::cout << "Loading files..." << std::endl;
    BitsetsDiff diff(file1, file2);
    BasicDiffResult* res = diff.compute_diff(dbtype, compname, true);
    //res->print_results();
    // res->write_results();
    res->write_results_machine();
    delete res;

    std::cout << "Done !" << std::endl;
    return 0;
}
