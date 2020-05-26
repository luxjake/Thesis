#include <cstring>
#include <omp.h>

#include "mem_data_manager.hpp"

void MemDataManager::open_database(const std::string path) {
    rocksdb::Options options;
    options.error_if_exists = true;
    options.create_if_missing = true;
    options.max_open_files = -1;
    options.compaction_style = rocksdb::kCompactionStyleLevel;
    options.compression = rocksdb::kLZ4Compression;
    options.bottommost_compression = rocksdb::kZSTD;
    rocksdb::Status status = rocksdb::DB::Open(options, path, &this->db);
    assert(status.ok());
    
    rocksdb::ColumnFamilyOptions cfopt;
    cfopt.compaction_style = rocksdb::kCompactionStyleLevel;
    cfopt.compression = rocksdb::kLZ4Compression;
    cfopt.bottommost_compression = rocksdb::kZSTD;
    status = db->CreateColumnFamily(cfopt, "component_to_compid", &component_to_compid);
    assert(status.ok());
    status = db->CreateColumnFamily(cfopt, "compid_to_component", &compid_to_component);
    assert(status.ok());

    this->path = path;
}

MemDataManager::MemDataManager(const std::string path) : compid(0) {
    open_database(path);
}

MemDataManager::MemDataManager() : compid(0) {
    this->path = "./database/";
    open_database(path);
}

MemDataManager::~MemDataManager() {
    delete component_to_compid;
    delete compid_to_component;
    
    delete db;
}

char* MemDataManager::serialize_id(const uint64_t id) {
    char* s = (char*) malloc(sizeof(uint64_t));
    std::memcpy(s, (char*)&id, sizeof(uint64_t));
    return s;
}

uint64_t MemDataManager::unserialize_id(const char* sid) {
    uint64_t id;
    std::memcpy(&id, sid, sizeof(uint64_t));
    return id;
}

std::string MemDataManager::get_component_value(const uint64_t compid) {
    std::string val;
    char* id = serialize_id(compid);
    rocksdb::Status s = db->Get(rocksdb::ReadOptions(), compid_to_component, rocksdb::Slice(id, sizeof(uint64_t)), &val);
    delete id;
    return val;
}

void MemDataManager::insert_triple(const std::string file, std::string s, std::string p, std::string o, uint64_t *sid, uint64_t *pid, uint64_t *oid) {
    std::string tmpsid;
    rocksdb::Status status = db->Get(rocksdb::ReadOptions(), component_to_compid, s, &tmpsid);
    bool snotfound = status.IsNotFound();
    
    std::string tmppid;
    status = db->Get(rocksdb::ReadOptions(), component_to_compid, p, &tmppid);
    bool pnotfound = status.IsNotFound();
    
    std::string tmpoid;
    status = db->Get(rocksdb::ReadOptions(), component_to_compid, o, &tmpoid);
    bool onotfound = status.IsNotFound();
    
    rocksdb::Slice slice_sid = rocksdb::Slice(tmpsid.c_str(), sizeof(uint64_t));

    if (snotfound || pnotfound || onotfound) { // some of the triple components are not known
        rocksdb::WriteBatch batch;
        char* ssid = nullptr;
        char* spid = nullptr;
        char* soid = nullptr;
        if (snotfound) {
            ssid = serialize_id(this->compid);
            *sid = this->compid;
            slice_sid = rocksdb::Slice(ssid, sizeof(uint64_t));
            batch.Put(component_to_compid, s, slice_sid);
            batch.Put(compid_to_component, slice_sid, s);
            this->compid++;
        } else {
            *sid = unserialize_id(tmpsid.c_str());
        }
        if (pnotfound) {
            spid = serialize_id(this->compid);
            *pid = this->compid;
            rocksdb::Slice tmpp(spid, sizeof(uint64_t));
            batch.Put(component_to_compid, p, tmpp);
            batch.Put(compid_to_component, tmpp, p);
            this->compid++;
        } else {
            *pid = unserialize_id(tmppid.c_str());
        }
        if (onotfound) {
            soid = serialize_id(this->compid);
            *oid = this->compid;
            rocksdb::Slice tmpo(soid, sizeof(uint64_t));
            batch.Put(component_to_compid, o, tmpo);
            batch.Put(compid_to_component, tmpo, o);
            this->compid++;
        } else {
            *oid = unserialize_id(tmpoid.c_str());
        }
        status = db->Write(rocksdb::WriteOptions(), &batch);
        if (ssid) delete ssid;
        if (spid) delete spid;
        if (soid) delete soid;
    } else { // all components were already in the db
        *sid = unserialize_id(tmpsid.c_str());
        *pid = unserialize_id(tmppid.c_str());
        *oid = unserialize_id(tmpoid.c_str());
    }

    insert_subjindex(file, *sid, *pid, *oid);
}

void MemDataManager::insert_subjindex(const std::string file, const uint64_t sid, const uint64_t pid, const uint64_t oid) {
    if (file == file1_name) {
        if (subject_index1.count(sid) == 0) {
            subject_index1.emplace(sid, std::vector<std::pair<uint64_t,uint64_t>>());
        }
        subject_index1[sid].emplace_back(std::make_pair(pid, oid));
    } else if (file == file2_name) {
        if (subject_index2.count(sid) == 0) {
            subject_index2.emplace(sid, std::vector<std::pair<uint64_t,uint64_t>>());
        }
        subject_index2[sid].emplace_back(std::make_pair(pid, oid));
    }    
}

uint64_t MemDataManager::delete_subjects(const std::string file, const std::vector<bool>& subjids) {
    uint64_t s_rem = 0;
    for (size_t i=0; i<subjids.size(); i++) {
        if (subjids[i]) {
            if (file == file1_name) {
                s_rem += subject_index1[i].size();
                subject_index1.erase(i);
            } else {
                s_rem += subject_index2[i].size();
                subject_index2.erase(i);
            }
        }
    }
    return s_rem;
}

void MemDataManager::delete_subject_pred(const std::string file, const uint64_t sid, const uint64_t pid) {
    if (file == file1_name) {
        if (subject_index1.count(sid) != 0) {
            for (auto it=subject_index1[sid].begin(); it!=subject_index1[sid].end();) {
                if ((*it).first == pid) {
                    it = subject_index1[sid].erase(it);
                } else {
                    it++;
                }
            }
        }
    } else {
        if (subject_index2.count(sid) != 0) {
            for (auto it=subject_index2[sid].begin(); it!=subject_index2[sid].end();) {
                if ((*it).first == pid) {
                    it = subject_index2[sid].erase(it);
                } else {
                    it++;
                }
            }
        }
    }   
}

void MemDataManager::build_subjpred_index(const std::string file) {
    if (file == file1_name) {
        for (auto v: subject_index1) {
            for (auto p: v.second) {
                SubjPredKey sp;
                sp.sid = v.first;
                sp.pid = p.first;
                if (subjpred_index1.count(sp) == 0) {
                    subjpred_index1.emplace(sp, std::vector<uint64_t>());
                }                    
                subjpred_index1[sp].emplace_back(p.second);
            }
        }
    } else {
        for (auto v: subject_index2) {
            for (auto p: v.second) {
                SubjPredKey sp;
                sp.sid = v.first;
                sp.pid = p.first;
                if (subjpred_index2.count(sp) == 0) {
                    subjpred_index2.emplace(sp, std::vector<uint64_t>());
                }                    
                subjpred_index2[sp].emplace_back(p.second);
            }
        }
    }
}

void MemDataManager::build_subjpred_indexes() {
    #pragma omp parallel sections
    {   
        #pragma omp section
        {
            build_subjpred_index(file1_name);
        }
        #pragma omp section
        {
            build_subjpred_index(file2_name);
        }
    }
}

std::vector<uint64_t>& MemDataManager::get_sp_value(const std::string file, const SubjPredKey key) { 
    if (file == file1_name) {
        if (subjpred_index1.count(key) == 0) {
            subjpred_index1.emplace(key, std::vector<uint64_t>());
        }
        return subjpred_index1[key];
    } else {
        if (subjpred_index2.count(key) == 0) { // we create a new, empty, vector if not found since we return a ref
            subjpred_index2.emplace(key, std::vector<uint64_t>());
        }
        return subjpred_index2[key];
    }
}

std::vector<std::pair<uint64_t,uint64_t>>& MemDataManager::get_s_value(const std::string file, uint64_t sid) {
    if (file == file1_name) {
        if (subject_index1.count(sid) == 0) {
            subject_index1.emplace(sid, std::vector<std::pair<uint64_t,uint64_t>>());
        }
        return subject_index1[sid];
    } else {
        if (subject_index2.count(sid) == 0) {
            subject_index2.emplace(sid, std::vector<std::pair<uint64_t,uint64_t>>());
        }
        return subject_index2[sid];
    }
}

std::unordered_map<SubjPredKey, std::vector<uint64_t>>& MemDataManager::get_sp_vec1() {
    return subjpred_index1;
}

std::unordered_map<SubjPredKey, std::vector<uint64_t>>& MemDataManager::get_sp_vec2() {
    return subjpred_index2;
}

void MemDataManager::set_file1_name(const std::string name) {
    file1_name = name;
}

void MemDataManager::set_file2_name(const std::string name) {
    file2_name = name;
}

uint64_t MemDataManager::get_compid() {
    return compid;
}