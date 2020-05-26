#include <iostream>

#include "high_diff_result.hpp"

HighDiffResult::HighDiffResult(std::string res_file) : 
    ntriples1(0), ntriples2(0), entities_changes(0), 
    entities_changes_triples(0), entity_adds(0), entity_dels(0),
    objects_updates(0), objects_additions(0), objects_deletions(0),
    total_components(0), added_components(0), deleted_components(0) {
    result_file = res_file + ".res";
}

HighDiffResult::~HighDiffResult() {
    // make sure to close all file descriptors
}

void HighDiffResult::increment_triples(const uint file_num) {
    if (file_num == 1) {
        ntriples1++;
    } else {
        ntriples2++;
    }
}

void HighDiffResult::increment_entity_triples(const uint64_t val) {
    entities_changes_triples += val;
}

void HighDiffResult::increment_added_components() {
    added_components++;
}

void HighDiffResult::increment_deleted_components() {
    deleted_components++;
}

void HighDiffResult::increment_entity_additions() {
    entity_adds++;
    entities_changes++;
}

void HighDiffResult::increment_entity_deletions() {
    entity_dels++;
    entities_changes++;
}

void HighDiffResult::increment_objects_updates(const uint64_t val) {
    objects_updates += val;
}

void HighDiffResult::increment_objects_additions(const uint64_t val) {
    objects_additions += val;
}

void HighDiffResult::increment_objects_deletions(const uint64_t val) {
    objects_deletions += val;
}

void HighDiffResult::write_results() {
    std::ofstream out;
    out.open(result_file, std::ios_base::app);
    std::string content = "Number of entities changes : " + std::to_string(entities_changes) + "\n"
                        + "Number of entity additions : " + std::to_string(entity_adds)
                        + "Number of entity deletions : " + std::to_string(entity_dels)
                        + "Number of objects updates : " + std::to_string(objects_updates) + "\n"
                        + "Number of objects additions : " + std::to_string(objects_additions) + "\n"
                        + "Number of objects deletions : " + std::to_string(objects_deletions) + "\n"
                        + "Number of components : " + std::to_string(total_components) + "\n"
                        + "Number of added components : " + std::to_string(added_components) + "\n"
                        + "Number of deleted components : " + std::to_string(deleted_components);

    out << content;
    out.close();
}

void HighDiffResult::write_results_machine() {
    std::ofstream out;
    out.open(result_file, std::ios_base::app);
    std::string content = std::to_string(ntriples1) + " "
                        + std::to_string(ntriples2) + " "
                        + std::to_string(entities_changes) + " "
                        + std::to_string(entity_adds) + " "
                        + std::to_string(entity_dels) + " "
                        + std::to_string(objects_updates) + " "
                        + std::to_string(objects_additions) + " "
                        + std::to_string(objects_deletions) + " "
                        + std::to_string(added_components) + " "
                        + std::to_string(deleted_components) + " "
                        + std::to_string(entities_changes_triples) + " "
                        + std::to_string(total_components);
    out << content;
    out.close();
}

void HighDiffResult::print_results() {
    std::string content = "Number of entities changes : " + std::to_string(entities_changes) + "\n"
                        + "Number of entity additions : " + std::to_string(entity_adds)
                        + "Number of entity deletions : " + std::to_string(entity_dels)
                        + "Number of objects updates : " + std::to_string(objects_updates) + "\n"
                        + "Number of objects additions : " + std::to_string(objects_additions) + "\n"
                        + "Number of objects deletions : " + std::to_string(objects_deletions);
    std::cout << content << std::endl;
}

void HighDiffResult::set_result_file(const std::string f) {
    result_file = f;
}

void HighDiffResult::set_total_components(const uint64_t val) {
    total_components = val;
}

// TODO
void HighDiffResult::write_entity(const std::string end, const bool is_add) {
    return;
}

void HighDiffResult::write_obj_updates(const std::string obj, const bool is_add) {
    return;
}

void HighDiffResult::write_obj(const std::string obj, const bool is_add) {
    return;
}