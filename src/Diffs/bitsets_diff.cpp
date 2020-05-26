#include <vector>
#include <ctime>
#include <omp.h>
#include <chrono>

#include "bitsets_diff.hpp"
#include "tsv_parser.hpp"
#include "para_parser.hpp"

BitsetsDiff::BitsetsDiff(std::string f1, std::string f2) {
    file1 = f1;
    file2 = f2;
}

std::pair<std::vector<bool>*, std::vector<bool>*>
BitsetsDiff::diff(const std::vector<bool>& v1, const std::vector<bool>& v2, u_int64_t max_id) {
    std::cout << "Files loaded, computing diff..." << std::endl;
    auto begin = std::chrono::high_resolution_clock::now();
    std::vector<bool>* addvec = new std::vector<bool>(max_id+1);
    std::vector<bool>* delvec = new std::vector<bool>(max_id+1);

    addvec->assign(max_id+1, false);
    delvec->assign(max_id+1, false);

    #pragma omp parallel for
    for (u_int64_t i=0; i<max_id; i++) {
        bool tmpv1 = v1[i];
        bool tmpv2 = v2[i];
        if (tmpv1 != tmpv2) {
            if (tmpv1) {
                (*delvec)[i] = true;
            } else {
                (*addvec)[i] = true;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::string dt = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end-begin).count()) + "ms";
    std::cout << "Diff computed in " << dt << std::endl;
    std::pair<std::vector<bool>*, std::vector<bool>*> p(addvec, delvec);
    return p;
}

BasicDiffResult* BitsetsDiff::compute_diff(std::string dbtype, std::string diff_name, bool write_add_del) {    

    // Creation of files names.
    std::string add_file = diff_name + ".add";
    std::string del_file = diff_name + ".del";
    std::string res_file = diff_name + ".res";
    std::string id_file = diff_name + "_identifiers.kch";
    std::string data_file = diff_name + "_data.kch";

    BasicDiffResult* res = nullptr;
    if (write_add_del) {
        res = new BasicDiffResult(res_file, add_file, del_file);
    } else {
        res = new BasicDiffResult(res_file);
    }

    std::string files_name = file1 + " " + file2 + "\n"; 
    res->write(files_name, res_file);

    Database* iddb = DiffStrategy::open_database(dbtype, id_file);
    Database* datadb = nullptr;
    if (write_add_del) {
        if (dbtype == "hybrid") {
            datadb = DiffStrategy::open_database("rocksdb", data_file);
        } else {
            datadb = DiffStrategy::open_database(dbtype, data_file);
        }        
    }

    SharedDatasetInfo sdi(iddb, datadb);
    std::vector<bool> vec1;
    std::vector<bool> vec2;

    Parser* p1;
    if (file1.find(".tsv") != std::string::npos) {
        p1 = new TSVParser(&vec1, &sdi, write_add_del);
        //p1 = new ParallelParser(&vec1, &sdi, write_add_del);
    } else if (file1.find(".nt") != std::string::npos) {
        p1 = new TSVParser(&vec1, &sdi, write_add_del);
        //p1 = new ParallelParser(&vec1, &sdi, write_add_del);
    } else {
        std::cerr << "Sorry, only TSV files are supported at the moment." << std::endl;
        return nullptr;
    }
    Parser* p2;
    if (file1.find(".tsv") != std::string::npos) {
        p2 = new TSVParser(&vec2, &sdi, write_add_del);
        //p2 = new ParallelParser(&vec2, &sdi, write_add_del);
    } else if (file1.find(".nt") != std::string::npos) {
        p2 = new TSVParser(&vec2, &sdi, write_add_del);
        //p2 = new ParallelParser(&vec2, &sdi, write_add_del);
    } else {
        std::cerr << "Sorry, only TSV files are supported at the moment." << std::endl;
        return nullptr;
    }

    uint64_t nf1 = 0;
    uint64_t nf2 = 0;

    #pragma omp parallel sections
    {   
        #pragma omp section
        {
            p1->read_file(file1);
            nf1 = p1->get_number_of_triples();
            delete p1;
        }
        #pragma omp section
        {
            p2->read_file(file2);
            nf2 = p2->get_number_of_triples();
            delete p2;
        }
    }
    
    res->set_nlines_file1(nf1);
    res->set_nlines_file2(nf2);

    uint64_t maxtriple_id = sdi.get_number_triples();
    if (vec1.size() < maxtriple_id) {
        TSVParser::increase_vector(&vec1, maxtriple_id);        
    }
    if (vec2.size() < maxtriple_id) {
        TSVParser::increase_vector(&vec2, maxtriple_id);        
    }

    //The type is : std::pair<std::vector<bool>*, std::vector<bool>*>
    auto adddel = diff(vec1, vec2, maxtriple_id);

    res->set_union_size(maxtriple_id);

    Database::DatabaseIterator* it = iddb->get_iterator();
    it->reset();
    while(it->has_next()) {
        std::pair<std::string, std::string> keyval = it->next();
        u_int64_t id = std::stoul(keyval.second);
        if (adddel.first->at(id)) {
            if (write_add_del) {
                std::string triple;
                if (!datadb->get_record(keyval.first, &triple)) {
                    return nullptr;
                }
                res->write_adddel(triple, "add");
            }
            res->increment_additions();
        } else if (adddel.second->at(id)) {
            if (write_add_del) {
                std::string triple;
                if (!datadb->get_record(keyval.first, &triple)) {
                    return nullptr;
                }
                res->write_adddel(triple, "del");
            }
            res->increment_deletions();
        }
    }

    delete adddel.first;
    delete adddel.second;

    delete iddb;
    delete datadb;

    return res;
}
