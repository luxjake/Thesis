#include <cstring>

#include "parser.hpp"
#include "data_manager.hpp"

void DataManager::open_database(const std::string path) {
    rocksdb::Options options;
    options.error_if_exists = true;
    options.create_if_missing = true;
    options.max_open_files = -1;
    //options.WAL_ttl_seconds = 0;
    //options.WAL_size_limit_MB = 0;
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
    status = db->CreateColumnFamily(cfopt, "subject_index1", &subject_index1);
    assert(status.ok());
    status = db->CreateColumnFamily(cfopt, "subject_index2", &subject_index2);
    assert(status.ok());
    status = db->CreateColumnFamily(cfopt, "subjpred_index1", &subjpred_index1);
    assert(status.ok());
    status = db->CreateColumnFamily(cfopt, "subjpred_index2", &subjpred_index2);
    assert(status.ok());

    this->path = path;
}

DataManager::DataManager(const std::string path) : compid(0) {
    open_database(path);
}

DataManager::DataManager() : compid(0) {
    this->path = "./database/";
    open_database(path);
}

DataManager::~DataManager() {
    delete component_to_compid;
    delete compid_to_component;
    delete subject_index1;
    delete subject_index2;
    delete subjpred_index1;
    delete subjpred_index2;
    
    for (auto h: handles) {
        delete h.second;
    }
    
    for (auto k: keys_sp1) {
        delete k;
    }
    for (auto k: keys_sp2) {
        delete k;
    }
    delete db;
}

char* DataManager::serialize_id(const uint64_t id) {
    char* s = (char*) malloc(sizeof(uint64_t));
    std::memcpy(s, (char*)&id, sizeof(uint64_t));
    return s;
}

uint64_t DataManager::unserialize_id(const char* sid) {
    uint64_t id;
    std::memcpy(&id, sid, sizeof(uint64_t));
    return id;
}

char* DataManager::serialize_vector(const std::vector<uint64_t>& v) {
    size_t s = v.size();
    char* vb = (char*) malloc(sizeof(uint64_t)*s+1);

    uint64_t suint = (uint64_t) s; // make sure that we have the size using the right number of bits
    std::memcpy(vb, (char*)&suint, sizeof(uint64_t));
    uint64_t offset = sizeof(uint64_t);
    for (uint64_t val: v) {
        std::memcpy(vb+offset, (char*)&val, sizeof(uint64_t));
        offset += sizeof(uint64_t);
    }

    return vb;
}

char* DataManager::serialize_pairvector(const std::vector<std::pair<uint64_t, uint64_t>>& v) {
    size_t s = v.size();
    char* vb = (char*) malloc((sizeof(uint64_t)*s*2)+1);

    uint64_t suint = (uint64_t) s;
    std::memcpy(vb, (char*)&suint, sizeof(uint64_t));
    uint64_t offset = sizeof(uint64_t);
    for (auto val: v) {
        std::memcpy(vb+offset, (char*)&val, sizeof(uint64_t)*2);
        offset += sizeof(uint64_t)*2;
    }

    return vb;
}

std::vector<uint64_t>* DataManager::unserialize_vector(const char* v) {
    std::vector<uint64_t>* vec = new std::vector<uint64_t>;
    uint64_t size;
    std::memcpy(&size, v, sizeof(uint64_t)); //get the number of elements
    uint64_t offset = sizeof(uint64_t);
    for (uint i=0; i<size; i++) {
        uint64_t tmp;
        std::memcpy(&tmp, v+offset, sizeof(uint64_t));
        vec->push_back(tmp);
        offset += sizeof(uint64_t);
    }

    return vec;
}

std::vector<std::pair<uint64_t, uint64_t>>* DataManager::unserialize_pairvector(const char* v) {
    std::vector<std::pair<uint64_t, uint64_t>>* vec = new std::vector<std::pair<uint64_t, uint64_t>>;
    uint64_t size;
    std::memcpy(&size, v, sizeof(uint64_t));
    uint64_t offset = sizeof(uint64_t);
    for (uint i=0; i<size; i++) {
        uint64_t tmp1;
        std::memcpy(&tmp1, v+offset, sizeof(uint64_t));
        offset += sizeof(uint64_t);
        uint64_t tmp2;
        std::memcpy(&tmp2, v+offset, sizeof(uint64_t));
        offset += sizeof(uint64_t);
        vec->emplace_back(std::make_pair(tmp1, tmp2));
    }

    return vec;
}

std::string DataManager::get_component_value(const uint64_t compid) {
    std::string val;
    char* id = serialize_id(compid);
    rocksdb::Status s = db->Get(rocksdb::ReadOptions(), compid_to_component, rocksdb::Slice(id, sizeof(uint64_t)), &val);
    delete id;
    return val;
}

void DataManager::insert_triple(const std::string file, std::string s, std::string p, std::string o, uint64_t *sid, uint64_t *pid, uint64_t *oid) {
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

void DataManager::insert_subjindex(const std::string file, const uint64_t sid, const uint64_t pid, const uint64_t oid) {
    rocksdb::ColumnFamilyHandle* cfhandle;
    
    if (file == file1_name) {
        cfhandle = subject_index1;
    } else {
        cfhandle = subject_index2;
    }
    
    char* csid = serialize_id(sid);
    rocksdb::Slice ssid(csid, sizeof(uint64_t));

    std::string val;
    rocksdb::Status s = db->Get(rocksdb::ReadOptions(), cfhandle, ssid, &val);
    if (s.IsNotFound()) {
        std::vector<std::pair<uint64_t, uint64_t>> vtmp;
        vtmp.emplace_back(std::make_pair(pid, oid));
        char* cv = serialize_pairvector(vtmp);
        size_t size = (sizeof(uint64_t)*vtmp.size()*2)+sizeof(uint64_t);
        db->Put(rocksdb::WriteOptions(), cfhandle, ssid, rocksdb::Slice(cv, size));
        delete cv;
    } else {
        std::vector<std::pair<uint64_t, uint64_t>>* vtmp = unserialize_pairvector(val.c_str());
        vtmp->emplace_back(std::make_pair(pid, oid));
        char* cv = serialize_pairvector((*vtmp));
        size_t size = (sizeof(uint64_t)*vtmp->size()*2)+sizeof(uint64_t);
        db->Put(rocksdb::WriteOptions(), cfhandle, ssid, rocksdb::Slice(cv, size));
        delete cv;
        delete vtmp;
    }

    delete csid;
}

uint64_t DataManager::delete_subjects(const std::string file, const std::vector<bool>& subjids) {
    uint64_t aff_triples = 0;
    
    rocksdb::ColumnFamilyHandle* cfhandle;
    
    if (file == file1_name) {
        cfhandle = subject_index1;
    } else {
        cfhandle = subject_index2;
    }
    
    for (size_t i=0; i<subjids.size(); i++) {
        if (subjids[i]) {
            char* id = serialize_id(i);
            std::string val;
            rocksdb::Slice sid(id, sizeof(size_t));
            rocksdb::Status s = db->Get(rocksdb::ReadOptions(), cfhandle, sid, &val);
            if (!s.IsNotFound()) {
                uint64_t size;
                std::memcpy(&size, val.c_str(), sizeof(uint64_t));
                aff_triples += size;
            }
            db->Delete(rocksdb::WriteOptions(), cfhandle, sid);
            delete id;
        }
    }

    return aff_triples;
}

void DataManager::delete_subject_pred(const std::string file, const uint64_t sid, const uint64_t pid) {
    rocksdb::ColumnFamilyHandle* cfhandle;
    
    if (file == file1_name) {
        cfhandle = subject_index1;
    } else {
        cfhandle = subject_index2;
    }

    char* ssid = serialize_id(sid);
    rocksdb::Slice slice_sid(ssid, sizeof(uint64_t));

    std::string values;
    rocksdb::Status status = db->Get(rocksdb::ReadOptions(), cfhandle, slice_sid, &values);

    if (!status.IsNotFound()) {
        if (values.size() > 1) {
            std::vector<std::pair<uint64_t,uint64_t>>* pairvec = unserialize_pairvector(values.c_str());

            for (auto it=pairvec->begin(); it!=pairvec->end();) {
                if ((*it).first == pid) {
                    it = pairvec->erase(it);
                } else {
                    it++;
                }
            }

            char* ser = serialize_pairvector((*pairvec));
            db->Put(rocksdb::WriteOptions(), cfhandle, slice_sid, rocksdb::Slice(ser, (sizeof(uint64_t)*pairvec->size()*2)+sizeof(uint64_t)));
            
            delete ser;
            delete pairvec;
        }            
    }
    delete ssid;    
}


void DataManager::build_subjpred_index(const std::string file) {
    rocksdb::ColumnFamilyHandle* cfhandle_subj;
    rocksdb::ColumnFamilyHandle* cfhandle_subjpred;

    if (file == file1_name) {
        cfhandle_subj = subject_index1;
        cfhandle_subjpred = subjpred_index1;
    } else {
        cfhandle_subj = subject_index2;
        cfhandle_subjpred = subjpred_index2;
    }

    rocksdb::Iterator* it = db->NewIterator(rocksdb::ReadOptions(), cfhandle_subj);
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        const char* s = it->key().data();
        std::vector<std::pair<uint64_t,uint64_t>>* po_vec = unserialize_pairvector(it->value().data());
        for (auto po: (*po_vec)) {
            char* p = serialize_id(po.first);
            char* key = (char*) malloc(sizeof(uint64_t)*2); // !!!!! -> need to be deleted at some point

            std::memcpy(key, s, sizeof(uint64_t));
            std::memcpy(key+sizeof(uint64_t), p, sizeof(uint64_t));
            
            rocksdb::Slice keyslice(key, sizeof(uint64_t)*2);
            if (file == file1_name) {
                keys_sp1.push_back(key);
            } else {
                keys_sp2.push_back(key);
            }            
            
            std::string o_vec_bytes;
            rocksdb::Status status = db->Get(rocksdb::ReadOptions(), cfhandle_subjpred, keyslice, &o_vec_bytes);
            std::vector<uint64_t>* o_vec;
            if (status.IsNotFound()) {
                o_vec = new std::vector<uint64_t>;
            } else {
                o_vec = unserialize_vector(o_vec_bytes.c_str());
            }            
            o_vec->emplace_back(po.second);
            char* new_o_vec_bytes = serialize_vector((*o_vec));

            db->Put(rocksdb::WriteOptions(), cfhandle_subjpred, keyslice, rocksdb::Slice(new_o_vec_bytes, sizeof(uint64_t)*o_vec->size()+sizeof(uint64_t)));
            
            delete p;
            delete new_o_vec_bytes;
            delete o_vec;
        }
        delete po_vec;  
    }

    delete it;
}

void DataManager::build_subjpred_indexes() {
    build_subjpred_index(file1_name);
    build_subjpred_index(file2_name);
}

const std::vector<char*>* DataManager::get_sp_keys(const std::string file) {
    if (file == file1_name) {
        return &keys_sp1;
    } else {
        return &keys_sp2;
    }
    return nullptr;
}

char* DataManager::get_sp_value(const std::string file, const rocksdb::Slice key) {
    rocksdb::ColumnFamilyHandle* cfhandle_subjpred;

    if (file == file1_name) {
        cfhandle_subjpred = subjpred_index1;
    } else {
        cfhandle_subjpred = subjpred_index2;
    }
    
    std::string val;
    rocksdb::Status status = db->Get(rocksdb::ReadOptions(), cfhandle_subjpred, key, &val);

    char* ret_val;
    if (status.IsNotFound()) {
        ret_val = nullptr;
    } else {
        ret_val = (char*) malloc(val.size());
        std::memcpy(ret_val, val.c_str(), val.size());
    }
    
    return ret_val;
}

void DataManager::set_sp_value(const std::string file, const rocksdb::Slice key, std::vector<uint64_t> values) {
    rocksdb::ColumnFamilyHandle* cfhandle_subjpred;

    if (file == file1_name) {
        cfhandle_subjpred = subjpred_index1;
    } else {
        cfhandle_subjpred = subjpred_index2;
    }

    char* val = serialize_vector(values);
    rocksdb::Status status = db->Put(rocksdb::WriteOptions(), cfhandle_subjpred, key, rocksdb::Slice(val, values.size()*sizeof(uint64_t)+sizeof(uint64_t)));
    assert(status.ok());
}

char* DataManager::get_s_value(const std::string file, uint64_t sid) {
    rocksdb::ColumnFamilyHandle* cfhandle;
    
    if (file == file1_name) {
        cfhandle = subject_index1;
    } else {
        cfhandle = subject_index2;
    }

    char* ssid = serialize_id(sid);
    rocksdb::Slice slice_sid(ssid, sizeof(uint64_t));
    std::string val;
    rocksdb::Status status = db->Get(rocksdb::ReadOptions(), cfhandle, slice_sid, &val);

    char* ret_val;
    if (status.IsNotFound()) {
        ret_val = nullptr;
    } else {
        ret_val = (char*) malloc(val.size());
        std::memcpy(ret_val, val.c_str(), val.size());
    }
    
    return ret_val;
}

bool DataManager::create_column_family(const std::string name) {
    rocksdb::ColumnFamilyHandle* cf;
    rocksdb::Status status = db->CreateColumnFamily(rocksdb::ColumnFamilyOptions(), name, &cf);
    handles.insert(std::make_pair(name, cf));
    return status.ok();
}

bool DataManager::delete_column_family(const std::string name) {
    rocksdb::Status s = db->DropColumnFamily(handles.at(name));
    delete handles.at(name);
    handles.erase(name);
    return s.ok();
}

rocksdb::ColumnFamilyHandle* DataManager::get_family_handle(const std::string name) {
    return handles.at(name);
}

void DataManager::set_file1_name(const std::string name) {
    file1_name = name;
}

void DataManager::set_file2_name(const std::string name) {
    file2_name = name;
}

uint64_t DataManager::get_compid() {
    return compid;
}