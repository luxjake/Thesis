#pragma once

#include "diff_result.hpp"
#include "kyoto_database.hpp"
#include "map_database.hpp"
#include "multi_kyoto_database.hpp"
#include "rocks_database.hpp"

class DiffStrategy {

    public:
        virtual DiffResult* compute_diff(std::string dbtype, std::string diff_name, bool write_add_del=true) = 0;
        virtual ~DiffStrategy() {}

        static Database* open_database(std::string dbtype, std::string dbname="") {
            if (dbtype == "onmemory" || dbtype == "hybrid") {
                return new MapDatabase();
            } else if (dbtype == "kcondisk") {
                if (dbname == "") {
                    return nullptr;
                }
                return new KCDatabase(dbname);
            } else if (dbtype == "multikcondisk") {
                if (dbname == "") {
                    return nullptr;
                }
                return new MultiKCDatabase(dbname);
            } else if (dbtype == "rocksdb") {
                if (dbname == "") {
                    return nullptr;
                }
                return new RocksDatabase(dbname);
            }
            std::cerr << "Unknown database type : " << dbtype << std::endl;
            return nullptr;
        }

};