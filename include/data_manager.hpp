#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "rocksdb/db.h"

class DataManager {

    public:
        DataManager();
        DataManager(const std::string path);
        ~DataManager();

        // Serialize an uint64_t id to string
        // Usefull to make an id into a byte stream usable for on disk DBs
        char* serialize_id(const uint64_t id);
        
        // Unserialize an uint64_t stored as byte stream
        // Only works if the string is the actual serialization of an uint64_t
        // Meaning that the byte stream must contains exactly 8 bytes
        uint64_t unserialize_id(const char* sid);

        // Serialize a vector of (uint64_t/pair<uint64_t,uint64_t>) into a byte stream
        // the first 8 bytes of the stream encode the size of the vector
        // the unserialize function read this value to know how much it need to read
        char* serialize_vector(const std::vector<uint64_t>& v);
        char* serialize_pairvector(const std::vector<std::pair<uint64_t, uint64_t>>& v);
        
        // Unserialize a byte stream representing a vector of (uint64_t/pair<uint64_t,uint64_t>)
        // the first 8 bytes (64bits) of the stream must encore the number of elements to decode
        // if this value is not set correctly, it will most likely lead to a segfault
        std::vector<uint64_t>* unserialize_vector(const char* v);
        std::vector<std::pair<uint64_t, uint64_t>>* unserialize_pairvector(const char* v);

        // Return the string representation of the component of id "compid"
        std::string get_component_value(const uint64_t compid);
                
        // Insert a triple in the database
        // file is the name of the file where the triple is from (depending on the name, insertion will be either in subject_index1 or subject_index2)
        // s, p, o correspond to the triple components
        // sid, pid, oid are pointers to uint64 values that will contains the ids assigned to each component
        void insert_triple(const std::string file, std::string s, std::string p, std::string o, uint64_t *sid, uint64_t *pid, uint64_t *oid);

        void insert_subjindex(const std::string file, const uint64_t sid, const uint64_t pid, const uint64_t oid);

        // Delete from the coresponding index the subjects whose ids are marked as true in subjids
        uint64_t delete_subjects(const std::string file, const std::vector<bool>& subjids);
        // Remove from the S->PO index all entry matching subject sid and containing the predicate pid
        void delete_subject_pred(const std::string file, const uint64_t sid, const uint64_t pid);

        // Create SP->O indexes based on the availables S->PO indexes  
        void build_subjpred_index(const std::string file);
        void build_subjpred_indexes();
        // Get all the SP couples, can be used to access data in SP->O indexes
        const std::vector<char*>* get_sp_keys(const std::string file);
        // Get a specific O for the file "file" with the key representing SP
        char* get_sp_value(const std::string file, const rocksdb::Slice key);
        void set_sp_value(const std::string file, const rocksdb::Slice key, std::vector<uint64_t> values);

        // Return the values stored in the S->PO index for the id sid
        char* get_s_value(const std::string file, uint64_t sid);

        bool create_column_family(const std::string name);
        bool delete_column_family(const std::string name);
        rocksdb::ColumnFamilyHandle* get_family_handle(const std::string name);

        void set_file1_name(const std::string name);
        void set_file2_name(const std::string name);

        uint64_t get_compid();

    protected:
        uint64_t compid;

        std::string path;

        std::string file1_name;
        std::string file2_name;

        void open_database(const std::string path);

        rocksdb::DB* db;
        rocksdb::ColumnFamilyHandle* component_to_compid;
        rocksdb::ColumnFamilyHandle* compid_to_component;

        // Need a separate subject_index for each DB
        rocksdb::ColumnFamilyHandle* subject_index1;
        rocksdb::ColumnFamilyHandle* subject_index2;

        rocksdb::ColumnFamilyHandle* subjpred_index1;
        rocksdb::ColumnFamilyHandle* subjpred_index2;
        //std::vector<rocksdb::Slice> keys_sp;
        std::vector<char*> keys_sp1;
        std::vector<char*> keys_sp2;

        // Map for additionnal handles
        std::unordered_map<std::string, rocksdb::ColumnFamilyHandle*> handles;
};