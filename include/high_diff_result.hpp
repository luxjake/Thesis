#pragma once

#include <fstream>

#include "diff_result.hpp"

class HighDiffResult : public DiffResult {

    public:
        HighDiffResult(std::string res_file);
        ~HighDiffResult();

        void increment_triples(const uint file_num);

        //void increment_entities_changes();
        void increment_entity_additions();
        void increment_entity_deletions();
        void increment_entity_triples(const uint64_t val);
        void increment_added_components();
        void increment_deleted_components();
        void increment_objects_updates(const uint64_t val);
        void increment_objects_additions(const uint64_t val);
        void increment_objects_deletions(const uint64_t val);

        void set_total_components(const uint64_t val);

        void write_results();
        void print_results();
        void write_results_machine();

        void write_entity(const std::string ent, const bool is_add);
        void write_obj_updates(const std::string obj, const bool is_add);
        void write_obj(const std::string obj, const bool is_add);

        void set_result_file(std::string file);

    private:
        uint64_t ntriples1;
        uint64_t ntriples2;

        uint64_t entities_changes;
        uint64_t entities_changes_triples;

        uint64_t entity_adds;
        uint64_t entity_dels;
        uint64_t objects_updates;
        uint64_t objects_additions;
        uint64_t objects_deletions;

        uint64_t total_components;
        uint64_t added_components;
        uint64_t deleted_components;

        std::string res_file;

};