#pragma once

#include <vector>

#include "data_manager.hpp"
#include "diff_strategy.hpp"
#include "high_diff_result.hpp"

// For the high level diff, we will use directly a rocksdb
// This will allow us to use compression
// and it open the possiblity for optimization
// as well as the use of "Column families" to store everything in the same database

class HighLevelDiff : DiffStrategy {

    public:
        HighLevelDiff(std::string file1, std::string file2);
        HighDiffResult* compute_diff(std::string dbtype, std::string diff_name, bool write_add_del=true);

        void entity_changes();
        void objects_updates();
        void add_del_objects();

    private:
        HighDiffResult* result;

        std::string file1;
        std::string file2;

        DataManager datamanager;
        
        std::vector<bool> subjects_db1;
        std::vector<bool> subjects_db2;

        std::vector<bool> components_db1;
        std::vector<bool> components_db2;

        void process_file(const std::string file);

        void diff(const std::vector<bool>& v1, const std::vector<bool>& v2, u_int64_t max_id, std::vector<bool>& vadd, std::vector<bool>& vdel);
        void diff(const std::vector<uint64_t>& v1, const std::vector<uint64_t>& v2, std::vector<uint64_t>& vadd, std::vector<uint64_t>& vdel);
        void diff(const std::vector<std::pair<uint64_t,uint64_t>>& v1, const std::vector<std::pair<uint64_t,uint64_t>>& v2, std::vector<std::pair<uint64_t,uint64_t>>& vadd, std::vector<std::pair<uint64_t,uint64_t>>& vdel);

        // Remove all values equals to "val" from vector "vec"
        void remove(std::vector<uint64_t>& vec, uint64_t val);

        uint64_t max(uint64_t v1, uint64_t v2, uint64_t v3);
};