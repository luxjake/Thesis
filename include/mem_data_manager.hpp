#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "rocksdb/db.h"

struct SubjPredKey
{
    uint64_t sid;
    uint64_t pid;

    bool operator==(const SubjPredKey& spk) const {
        return ((sid == spk.sid) && (pid == spk.pid));
    }
};

// Specialized hash for SubjPredKey
namespace std {
    template<>
    struct hash<SubjPredKey>
    {
        std::size_t operator()(const SubjPredKey& sp) const noexcept {
            const size_t h1 = std::hash<uint64_t>()(sp.pid);
            const size_t h2 = std::hash<uint64_t>()(sp.sid);
            return h1 ^ (h2 << 1);
        }
    };
}

class MemDataManager {

    public:       
        
        MemDataManager();
        MemDataManager(const std::string path);
        ~MemDataManager();

        // Serialize an uint64_t id to string
        // Usefull to make an id into a byte stream usable for on disk DBs
        char* serialize_id(const uint64_t id);
        
        // Unserialize an uint64_t stored as byte stream
        // Only works if the string is the actual serialization of an uint64_t
        // Meaning that the byte stream must contains exactly 8 bytes
        uint64_t unserialize_id(const char* sid);

        // Return the string representation of the component of id "compid"
        std::string get_component_value(const uint64_t compid);
                
        // Insert a triple in the database
        // file is the name of the file where the triple is from (depending on the name, insertion will be either in subject_index1 or subject_index2)
        // s, p, o correspond to the triple components
        // sid, pid, oid are pointers to uint64 values that will contains the ids assigned to each component
        void insert_triple(const std::string file, std::string s, std::string p, std::string o, uint64_t *sid, uint64_t *pid, uint64_t *oid);

        void insert_subjindex(const std::string file, const uint64_t sid, const uint64_t pid, const uint64_t oid);

        // Delete from the coresponding index the subjects whose ids are marked as true in subjids
        // return the number of deleted triples
        uint64_t delete_subjects(const std::string file, const std::vector<bool>& subjids);
        // Remove from the S->PO index all entry matching subject sid and containing the predicate pid
        void delete_subject_pred(const std::string file, const uint64_t sid, const uint64_t pid);

        // Create SP->O indexes based on the availables S->PO indexes  
        void build_subjpred_index(const std::string file);
        void build_subjpred_indexes();
        
        std::unordered_map<SubjPredKey, std::vector<uint64_t>>& get_sp_vec1(); 
        std::unordered_map<SubjPredKey, std::vector<uint64_t>>& get_sp_vec2(); 

        // Get a specific O for the file "file" with the key representing SP
        std::vector<uint64_t>& get_sp_value(const std::string file, const SubjPredKey key);

        // Return the values stored in the S->PO index for the id sid
        std::vector<std::pair<uint64_t,uint64_t>>& get_s_value(const std::string file, uint64_t sid);

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
        std::unordered_map<uint64_t, std::vector<std::pair<uint64_t,uint64_t>>> subject_index1;
        std::unordered_map<uint64_t, std::vector<std::pair<uint64_t,uint64_t>>> subject_index2;

        std::unordered_map<SubjPredKey, std::vector<uint64_t>> subjpred_index1;
        std::unordered_map<SubjPredKey, std::vector<uint64_t>> subjpred_index2;

};