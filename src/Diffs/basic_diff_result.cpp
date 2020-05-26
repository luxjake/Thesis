#include <iostream>

#include "basic_diff_result.hpp"

BasicDiffResult::BasicDiffResult(std::string res_file) {
    result_file = res_file;
    additions = 0;
    deletions = 0;
    nlines_file1 = 0;
    nlines_file2 = 0;
    union_size = 0;
}

BasicDiffResult::BasicDiffResult(std::string res_file, std::string add_file, std::string del_file)
    : add_file(add_file, std::ios_base::out), del_file(del_file, std::ios_base::out) {
    result_file = res_file;
    additions = 0;
    deletions = 0;
    nlines_file1 = 0;
    nlines_file2 = 0;
    union_size = 0;
}


void BasicDiffResult::increment_additions() {
    additions++;
}

void BasicDiffResult::increment_deletions() {
    deletions++;
}

void BasicDiffResult::set_nlines_file1(uint64_t n) {
    nlines_file1 = n;
}

void BasicDiffResult::set_nlines_file2(uint64_t n) {
    nlines_file2 = n;
}

void BasicDiffResult::set_union_size(const uint64_t s) {
    union_size = s;
}

void BasicDiffResult::print_results() {
    std::cout << "Number of additions : " << additions << std::endl;
    std::cout << "Number of deletions : " << deletions << std::endl;
}

void BasicDiffResult::write_results() {
    float p_add = ((float)additions / (float)nlines_file2) * 100.0;
    float p_del = ((float)deletions / (float)nlines_file1) * 100.0;

    std::ofstream out;
    out.open(result_file, std::ios_base::app);
    std::string file_content = "Number of additions : " 
                            + std::to_string(additions)
                            + "\nPercentage of additions in newest file : "
                            + std::to_string(p_add) + "%" + " (Number of triple in newest file : " + std::to_string(nlines_file2) + ")"
                            + "\nNumber of deletions : "
                            + std::to_string(deletions)
                            + "\nPercentage of deleted triples from oldest file : "
                            + std::to_string(p_del) + "%" + " (Number of triple in oldest file : " + std::to_string(nlines_file1) + ")"
                            + "\nSize of the union :"
                            + std::to_string(union_size)
                            + "\n";

    out << file_content;
    out.close();
}

void BasicDiffResult::write_results_machine() {
    if (nlines_file2 == 0) {
        nlines_file2 = 1;
    }
    if (nlines_file1 == 0) {
        nlines_file1 = 1;
    } 
    float p_add = ((float)additions / (float)nlines_file2) * 100.0;
    float p_del = ((float)deletions / (float)nlines_file1) * 100.0;

    std::ofstream out;
    out.open(result_file, std::ios_base::app);
    std::string line = std::to_string(additions) 
                        + " " 
                        + std::to_string(deletions)
                        + " "
                        + std::to_string(nlines_file2)
                        + " "
                        + std::to_string(nlines_file1)
                        + " "
                        + std::to_string(p_add)
                        + " "
                        + std::to_string(p_del)
                        + " "
                        + std::to_string(union_size);
    
    out << line;
    out.close();
}

void BasicDiffResult::write(std::string triple, std::string file) {
    std::ofstream out;
    out.open(file, std::ios_base::app);
    out << triple << std::endl;
    out.close();
}

void BasicDiffResult::write_adddel(std::string triple, std::string ad) {
    if (ad == "add") {
        if (!add_file.is_open()) {
            add_file.open(add_file_name, std::ios_base::out);
        }
        add_file << triple << std::endl;
        return;
    } else if (ad == "del") {
        if (!del_file.is_open()) {
            del_file.open(del_file_name, std::ios_base::out);
        }
        del_file << triple << std::endl;
        return;
    } else {
        std::cerr << "Wrong file. Use this function with \"add\" or \"del\"" << std::endl;
        return;
    }
}

void BasicDiffResult::set_result_file(std::string f) {
    this->result_file = f;
}

BasicDiffResult::~BasicDiffResult() {
    if (add_file.is_open()) {
        add_file.close();
    }
    if (del_file.is_open()) {
        del_file.close();
    }
}
