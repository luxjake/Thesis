#include <fstream>
#include <algorithm>
#include <cmath>
#include <omp.h>

#include "mem_high_level_diff.hpp"
#include "parser.hpp"

MemHighLevelDiff::MemHighLevelDiff(std::string file1, std::string file2) {
    result = nullptr; // no result object by default

    this->file1 = file1;
    this->file2 = file2;
}

void MemHighLevelDiff::diff(const std::vector<bool>& v1, const std::vector<bool>& v2, u_int64_t max_id, std::vector<bool>& vadd, std::vector<bool>& vdel) {

    vadd.assign(max_id, false);
    vdel.assign(max_id, false);

    //#pragma omp parallel for
    for (size_t i=0; i<max_id; i++) {
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

void MemHighLevelDiff::diff(const std::vector<uint64_t>& v1, const std::vector<uint64_t>& v2, std::vector<uint64_t>& vadd, std::vector<uint64_t>& vdel) {
    // #pragma omp parallel for
    for (size_t i=0; i<v1.size(); i++) {
        auto f = std::find(v2.begin(), v2.end(), v1[i]);
        if (f == v2.end()) { // not found : deletion
            // #pragma omp critical(diff2_del)
            // {
            //     vdel.push_back(v1[i]);
            // }
            vdel.push_back(v1[i]);
        }
    }
    // #pragma omp parallel for
    for (size_t i=0; i<v2.size(); i++) {
        auto f = std::find(v1.begin(), v1.end(), v2[i]);
        if (f == v1.end()) { // not found : deletion
            // #pragma omp critical(diff2_add)
            // {
            //     vadd.push_back(v2[i]);
            // }
            vadd.push_back(v2[i]);
        }
    }
}

void MemHighLevelDiff::diff(const std::vector<std::pair<uint64_t,uint64_t>>& v1, const std::vector<std::pair<uint64_t,uint64_t>>& v2, std::vector<std::pair<uint64_t,uint64_t>>& vadd, std::vector<std::pair<uint64_t,uint64_t>>& vdel) {
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

uint64_t MemHighLevelDiff::max(uint64_t v1, uint64_t v2, uint64_t v3) {
    uint64_t rv=v1;
    if (v2 > rv) rv = v2;
    if (v3 > rv) rv = v3;
    return rv;
}

void MemHighLevelDiff::process_file(const std::string file) {

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
        #pragma omp critical(insert_triple) // to ensure proper id assignements this must be critical in multithreaded runs
        {
            datamanager.insert_triple(file, s, p, o, &sid, &pid, &oid);
        }            
        uint64_t maxid = max(sid, pid, oid);
        if (file == file1) {
            #pragma omp critical(modify_vectors1) 
            {
                Parser::increase_vector(&subjects_db1, sid+1);            
                Parser::increase_vector(&components_db1, maxid+1);
                subjects_db1[sid] = true;
                components_db1[sid] = true;
                components_db1[pid] = true;
                components_db1[oid] = true;
            }            
        } else {
            #pragma omp critical(modify_vectors2)
            {
                Parser::increase_vector(&subjects_db2, sid+1);
                Parser::increase_vector(&components_db2, maxid+1);
                subjects_db2[sid] = true;
                components_db2[sid] = true;
                components_db2[pid] = true;
                components_db2[oid] = true;
            }            
        }

        if (result) {
            #pragma omp critical(increment_triples)
            {
                if (file == file1) {
                    result->increment_triples(1);
                } else {
                    result->increment_triples(2);
                }
            }            
        }
    }
    f.close();
}

void MemHighLevelDiff::compute_entities_changes() {
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
}

void MemHighLevelDiff::compute_objects_updates() {
    datamanager.build_subjpred_indexes();
    std::unordered_map<SubjPredKey, std::vector<uint64_t>>& sp_vec1 = datamanager.get_sp_vec1();
    std::unordered_map<SubjPredKey, std::vector<uint64_t>>& sp_vec2 = datamanager.get_sp_vec2();

    std::cout << "DEBUG --- MemHighLevelDiff::compute_objects_updates : SP vectors acquired..." << std::endl;

    uint64_t nupdates= 0;
    
    // Build key vector (memory usage... :( ) for parallel iteration later
    // OpenMP cannot figure out how to iterate the unordered_map otherwise
    // Is building this vector too costly ?
    std::vector<SubjPredKey> key_vec;
    for (auto sp: sp_vec1) {
        key_vec.emplace_back(sp.first);
    }

    std::vector<std::pair<SubjPredKey, uint64_t>> to_del_in_v1;
    std::vector<SubjPredKey> sp_to_del1;
    std::vector<std::pair<SubjPredKey, uint64_t>> to_del_in_v2;
    std::vector<SubjPredKey> sp_to_del2;

    #pragma omp parallel for
    for (size_t i=0; i<key_vec.size(); i++) {
        SubjPredKey k = key_vec[i];
        if (!sp_vec1[k].empty() && !sp_vec2[k].empty()) {
            std::vector<uint64_t> vadd;
            std::vector<uint64_t> vdel;
            diff(sp_vec1[k], sp_vec2[k], vadd, vdel);
            nupdates += vadd.size() + std::abs(int64_t(vadd.size()-vdel.size()));
            if (vadd.size() > 0 || vdel.size() > 0) {
                for (uint64_t v: vadd) {
                    #pragma omp critical(remove_vadd)
                    {
                        to_del_in_v1.push_back(std::make_pair(k,v));
                    }                    
                }
                for (auto v: vdel) {
                    #pragma omp critical(remove_vdel)
                    {
                        to_del_in_v2.push_back(std::make_pair(k,v));
                    }
                }
            }
                        
            //remove them from s->po index
            #pragma omp critical(del_subj_pred_f1)
            {
                sp_to_del1.push_back(k);
            }
            #pragma omp critical(del_subj_pred_f2)
            {
                sp_to_del2.push_back(k);
            }
        }
    }

    std::cout << "DEBUG --- MemHighLevelDiff::compute_objects_updates : starting deletions process..." << std::endl;

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (auto p: to_del_in_v1) {
                remove(sp_vec1[p.first], p.second);
            }
        }
        #pragma omp section
        {
            for (auto k: sp_to_del1) {
                datamanager.delete_subject_pred(file1, k.sid, k.pid);
            }
        }
        #pragma omp section
        {
            for (auto p: to_del_in_v2) {
                remove(sp_vec2[p.first], p.second);
            }
        }
        #pragma omp section
        {            
            for (auto k: sp_to_del2) {
                datamanager.delete_subject_pred(file2, k.sid, k.pid);
            }
        }
    }

    std::cout << "DEBUG --- MemHighLevelDiff::compute_objects_updates : deletions process finished !" << std::endl;

    result->increment_objects_updates(nupdates);
}

void MemHighLevelDiff::compute_objects_adddel() {
    uint64_t newobjs = 0;
    uint64_t delobjs = 0;
    for (size_t i=0; i<subjects_db1.size(); i++) {
        if (subjects_db1[i] && subjects_db2[i]) {
            auto po1 = datamanager.get_s_value(file1, i);  
            auto po2 = datamanager.get_s_value(file2, i);
  
            std::vector<std::pair<uint64_t,uint64_t>> poadd;
            std::vector<std::pair<uint64_t,uint64_t>> podel;
            diff(po1, po2, poadd, podel);
            newobjs += poadd.size();
            delobjs += podel.size();
        }
    }
    result->increment_objects_additions(newobjs);
    result->increment_objects_deletions(delobjs);
}

HighDiffResult* MemHighLevelDiff::compute_diff(std::string dbtype, std::string diff_name, bool write_add_del) {
    if (!result) result = new HighDiffResult(diff_name);
    
    datamanager.set_file1_name(file1);
    datamanager.set_file2_name(file2);

    std::cout << "Loading files..." << std::endl;
    
    #pragma omp parallel sections
    {   
        #pragma omp section
        {
            process_file(file1);
        }
        #pragma omp section
        {
            process_file(file2);
        }
    }

    result->set_total_components(datamanager.get_compid());

    // Entity changes ==================================================
    std::cout << "Computing Entity changes..." << std::endl;

    compute_entities_changes();
    
    // Objects updates =================================================
    std::cout << "Computing objects updates..." << std::endl;

    compute_objects_updates();
    
    // Add/Del Objects ===================================================

    std::cout << "Computing objects additions and deletions..." << std::endl;
    
    compute_objects_adddel();

    return result;
}

void MemHighLevelDiff::remove(std::vector<uint64_t>& vec, uint64_t val) {
    vec.erase(std::remove(vec.begin(), vec.end(), val), vec.end());
}