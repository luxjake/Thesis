#include <omp.h>
#include <fstream>
#include <algorithm>
#include <cmath>

#include "high_level_diff.hpp"
#include "parser.hpp"

HighLevelDiff::HighLevelDiff(std::string file1, std::string file2) {
    result = nullptr; // no result object by default

    this->file1 = file1;
    this->file2 = file2;
}

void HighLevelDiff::diff(const std::vector<bool>& v1, const std::vector<bool>& v2, u_int64_t max_id, std::vector<bool>& vadd, std::vector<bool>& vdel) {

    vadd.assign(max_id+1, false);
    vdel.assign(max_id+1, false);

    #pragma omp parallel for
    for (u_int64_t i=0; i<max_id; i++) {
        bool tmpv1 = v1[i];
        bool tmpv2 = v2[i];
        if (tmpv1 != tmpv2) {
            if (tmpv1) {
                vdel[i] = true;
            } else {
                vadd[i] = true;
            }
        }
    }

}

void HighLevelDiff::diff(const std::vector<uint64_t>& v1, const std::vector<uint64_t>& v2, std::vector<uint64_t>& vadd, std::vector<uint64_t>& vdel) {
    #pragma omp parallel for
    for (size_t i=0; i<v1.size(); i++) {
        auto f = std::find(v2.begin(), v2.end(), v1[i]);
        if (f == v2.end()) { // not found : deletion
            #pragma omp critical(diff2_del)
            {
                vdel.push_back(v1[i]);
            }
        }
    }
    #pragma omp parallel for
    for (size_t i=0; i<v2.size(); i++) {
        auto f = std::find(v1.begin(), v1.end(), v2[i]);
        if (f == v1.end()) { // not found : deletion
            #pragma omp critical(diff2_add)
            {
                vadd.push_back(v2[i]);
            }
        }
    }
}

void HighLevelDiff::diff(const std::vector<std::pair<uint64_t,uint64_t>>& v1, const std::vector<std::pair<uint64_t,uint64_t>>& v2, std::vector<std::pair<uint64_t,uint64_t>>& vadd, std::vector<std::pair<uint64_t,uint64_t>>& vdel) {
    #pragma omp parallel for
    for (size_t i=0; i<v1.size(); i++) {
        auto f = std::find(v2.begin(), v2.end(), v1[i]);
        if (f == v2.end()) { // not found : deletion
            #pragma omp critical(diff2_del)
            {
                vdel.push_back(v1[i]);
            }
        }
    }
    #pragma omp parallel for
    for (size_t i=0; i<v2.size(); i++) {
        auto f = std::find(v1.begin(), v1.end(), v2[i]);
        if (f == v1.end()) { // not found : deletion
            #pragma omp critical(diff2_add)
            {
                vadd.push_back(v2[i]);
            }
        }
    }
}

uint64_t HighLevelDiff::max(uint64_t v1, uint64_t v2, uint64_t v3) {
    uint64_t rv=v1;
    if (v2 > rv) rv = v2;
    if (v3 > rv) rv = v3;
    return rv;
}

void HighLevelDiff::process_file(const std::string file) {

    std::ifstream f(file, std::ifstream::in);
    if (f.fail()) {
        std::cerr << "The file " << file << " does not exist." << std::endl;
        return;
    }

    std::string delimiter;
    delimiter = Parser::get_delimiter(file);

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) {
            continue;
        }

        std::string s, p, o;
        Parser::parse_triple(line, delimiter, s, p, o);

        uint64_t sid, pid, oid;
        datamanager.insert_triple(file, s, p, o, &sid, &pid, &oid);
        uint64_t maxid = max(sid, pid, oid);
        if (file == file1) {
            Parser::increase_vector(&subjects_db1, sid+1);
            subjects_db1[sid] = true;
            Parser::increase_vector(&components_db1, maxid+1);
            components_db1[sid] = true;
            components_db1[pid] = true;
            components_db1[oid] = true;
        } else {
            Parser::increase_vector(&subjects_db2, sid+1);
            subjects_db2[sid] = true;
            Parser::increase_vector(&components_db2, maxid+1);
            components_db2[sid] = true;
            components_db2[pid] = true;
            components_db2[oid] = true;
        }

        if (result) {
            if (file == file1) {
                result->increment_triples(1);
            } else {
                result->increment_triples(2);
            }
        }
    }
    f.close();
}

HighDiffResult* HighLevelDiff::compute_diff(std::string dbtype, std::string diff_name, bool write_add_del) {
    if (!result) result = new HighDiffResult(diff_name);

    datamanager.set_file1_name(file1);
    datamanager.set_file2_name(file2);
    
    std::cout << "Loading files..." << std::endl;
    process_file(file1);
    process_file(file2);

    // Entity changes ==================================================
    std::cout << "Computing Entity changes..." << std::endl;

    result->set_total_components(datamanager.get_compid());

    size_t size_svec1 = subjects_db1.size();
    size_t size_svec2 = subjects_db2.size();
    if (size_svec1 != size_svec2) {
        if (size_svec1 < size_svec2) {
            Parser::increase_vector(&subjects_db1, size_svec2);
        } else {
            Parser::increase_vector(&subjects_db2, size_svec1);
        }
    }

    size_t size_cvec1 = components_db1.size();
    size_t size_cvec2 = components_db2.size();
    if (size_cvec1 != size_cvec2) {
        if (size_cvec1 < size_cvec2) {
            Parser::increase_vector(&components_db1, size_cvec2);
        } else {
            Parser::increase_vector(&components_db2, size_cvec1);
        }
    }

    uint64_t maxid = subjects_db1.size();
    std::vector<bool> subjadd;
    std::vector<bool> subjdel;
    diff(subjects_db1, subjects_db2, maxid, subjadd, subjdel);

    uint64_t maxcompid = components_db1.size();
    std::vector<bool> compadd;
    std::vector<bool> compdel;
    diff(components_db1, components_db2, maxcompid, compadd, compdel);

    for (size_t i=0; i<maxcompid; i++) {
        if (compdel[i]) {
            result->increment_deleted_components();
        } 
        if (compadd[i]) {
            result->increment_added_components();
        } 
    }
   
    uint64_t del_t = datamanager.delete_subjects(file1, subjdel);
    uint64_t add_t = datamanager.delete_subjects(file2, subjadd);
    result->increment_entity_triples(del_t);
    result->increment_entity_triples(add_t);

    for (size_t i=0; i<maxid; i++) {
        if (subjdel[i]) result->increment_entity_deletions();
        if (subjadd[i]) result->increment_entity_additions();

        subjects_db1[i] = subjdel[i] ? false : subjects_db1[i];
        subjects_db2[i] = subjadd[i] ? false : subjects_db2[i];
    }
    
    // Objects updates =================================================
    std::cout << "Computing objects updates..." << std::endl;

    datamanager.build_subjpred_indexes();
    const std::vector<char*>* keys = datamanager.get_sp_keys(file1);

    uint64_t nupdates= 0;
    for (auto key: *keys) {
        uint64_t tmps;
        std::memcpy(&tmps, key, sizeof(uint64_t));
        uint64_t tmpp;
        std::memcpy(&tmpp, key+sizeof(uint64_t), sizeof(uint64_t));

        rocksdb::Slice keyslice(key, sizeof(uint64_t)*2);
        char* o1 = datamanager.get_sp_value(file1, keyslice);
        char* o2 = datamanager.get_sp_value(file2, keyslice);
        
        if (o1!=nullptr && o2!=nullptr) {
            //std::cout << "obj updates compute" << std::endl;
            std::vector<uint64_t>* vo1 = datamanager.unserialize_vector(o1);
            std::vector<uint64_t>* vo2 = datamanager.unserialize_vector(o2);

            std::vector<uint64_t> vadd;
            std::vector<uint64_t> vdel;
            diff((*vo1), (*vo2), vadd, vdel);
            nupdates += vadd.size() + std::abs(int64_t(vadd.size()-vdel.size()));
            if (vadd.size() > 0 || vdel.size() > 0) {
                for (auto v: vadd) {
                    remove((*vo2), v);
                }

                for (auto v: vdel) {
                    remove((*vo1), v);
                }
            }
            
            datamanager.set_sp_value(file1, keyslice, (*vo1));
            datamanager.set_sp_value(file2, keyslice, (*vo2));
            
            //get subjects id (first 64bits of key) and pred id (second 64bits)
            char* subj_id_bytes = (char*) malloc(sizeof(uint64_t));
            char* pred_id_bytes = (char*) malloc(sizeof(uint64_t));

            std::memcpy(subj_id_bytes, key, sizeof(uint64_t));            
            std::memcpy(pred_id_bytes, key+sizeof(uint64_t), sizeof(uint64_t));
            
            uint64_t subj_id = datamanager.unserialize_id(subj_id_bytes);
            uint64_t pred_id = datamanager.unserialize_id(pred_id_bytes);
            
            //remove them from s->po index
            datamanager.delete_subject_pred(file1, subj_id, pred_id);
            datamanager.delete_subject_pred(file2, subj_id, pred_id);
            
            delete vo1;
            delete vo2;
            delete subj_id_bytes;
            delete pred_id_bytes;
        }
        
        delete o1;
        delete o2;
    }
    result->increment_objects_updates(nupdates);

    // Add/Del Objects ===================================================

    std::cout << "Computing objects additions and deletions..." << std::endl;
    
    uint64_t newobjs = 0;
    uint64_t delobjs = 0;
    for (size_t i=0; i<maxid; i++) {
        if (subjects_db1[i] && subjects_db2[i]) {
            char* po1 = datamanager.get_s_value(file1, i);
            std::vector<std::pair<uint64_t,uint64_t>>* po1_vec;
            if (po1) {
                po1_vec = datamanager.unserialize_pairvector(po1);
                delete po1;
            } else {
                po1_vec = new std::vector<std::pair<uint64_t,uint64_t>>;
            }                     
            char* po2 = datamanager.get_s_value(file2, i);
            std::vector<std::pair<uint64_t,uint64_t>>* po2_vec;
            if (po2) {
                po2_vec = datamanager.unserialize_pairvector(po2);
                delete po2;
            } else {
                po2_vec = new std::vector<std::pair<uint64_t,uint64_t>>;
            }     
            std::vector<std::pair<uint64_t,uint64_t>> poadd;
            std::vector<std::pair<uint64_t,uint64_t>> podel;
            diff((*po1_vec), (*po2_vec), poadd, podel);
            delete po1_vec;
            delete po2_vec;
            newobjs += poadd.size();
            delobjs += podel.size();
        }
    }
    result->increment_objects_additions(newobjs);
    result->increment_objects_deletions(delobjs);

    return result;
}

void HighLevelDiff::remove(std::vector<uint64_t>& vec, uint64_t val) {
    vec.erase(std::remove(vec.begin(), vec.end(), val), vec.end());
}