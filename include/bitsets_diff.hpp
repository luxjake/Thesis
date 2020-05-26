#pragma once

#include <vector>
#include <unordered_map>

#include "diff_strategy.hpp"
#include "basic_diff_result.hpp"

class BitsetsDiff : DiffStrategy {

    public:        
        BitsetsDiff(std::string file1, std::string file2);
        BasicDiffResult* compute_diff(std::string dbtype, std::string diff_name, bool write_add_del=true);

    private:
        std::string file1;
        std::string file2;

        std::vector<bool> vec1;
        std::vector<bool> vec2;

        //std::pair<std::reference_wrapper<std::vector<bool>>, std::reference_wrapper<std::vector<bool>>> 
          //          diff(const std::vector<bool>& v1, const std::vector<bool>& v2, u_int64_t max_id);

        std::pair<std::vector<bool>*, std::vector<bool>*> 
            diff(const std::vector<bool>& v1, const std::vector<bool>& v2, u_int64_t max_id);

};