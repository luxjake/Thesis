#include <iostream>

#include "mem_high_level_diff.hpp"
#include "high_diff_result.hpp"

int main(int argc, char **argv) {

    if (argc != 4) {
        std::cerr << "3 arguments needed :\n- the two files to compare\n- comparison name (use for the result file)" << std::endl;
        return -1;
    }

    std::string file1 = argv[1];
    std::string file2 = argv[2];
    std::string compname = argv[3];

    MemHighLevelDiff diff(file1, file2);
    HighDiffResult* res = diff.compute_diff("", compname, true);

    res->write_results_machine();
    //res->write_results();
    delete res;

    std::cout << "Done !" << std::endl;

}
