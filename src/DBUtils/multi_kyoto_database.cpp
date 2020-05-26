#include "multi_kyoto_database.hpp"

MultiKCDatabase::MultiKCDatabase(std::string path) : ndb(32) {
    
    for (unsigned i=0; i<ndb; i++) {
        std::string tmppath = std::to_string(i) + path;
        
        kyotocabinet::PolyDB* db = new kyotocabinet::PolyDB();    
        if (!db->open(tmppath, kyotocabinet::PolyDB::OWRITER | kyotocabinet::PolyDB::OCREATE)) {
            std::cerr << "open error: " << db->error().name() << std::endl;
            std::cerr << "file : " << path << std::endl;
            return;
        }
        this->db.push_back(db);
        this->cursors.push_back(db->cursor());
    }
    
    this->it = new MultiKCIterator(this, &cursors);
}

MultiKCDatabase::~MultiKCDatabase() {
    for (auto d: db) {
        if (!d->close()) {
            std::cerr << "close error: " << d->error().name() << std::endl;
        }
    }
    
    delete this->it;
}

std::pair<std::string, bool> MultiKCDatabase::add_record(std::string key, std::string value) {
    size_t k = std::stoul(key);
    unsigned dbid = k%ndb;
    if (!db[dbid]->add(key, value)) {
        std::string val;
        get_record(key, &val);
        return std::make_pair(val, false);
    }
    return std::make_pair(value, true);
}

void MultiKCDatabase::fast_add_record(std::string key, std::string value) {
    size_t k = std::stoul(key);
    unsigned dbid = k%ndb;
    db[dbid]->add(key, value);
}

bool MultiKCDatabase::remove_record(std::string key) {
    size_t k = std::stoul(key);
    unsigned dbid = k%ndb;
    if (!db[dbid]->remove(key)) {
        std::cerr << "remove error: " << db[dbid]->error().name() << std::endl;
        return false;
    }
    return true;
}

bool MultiKCDatabase::exist(std::string key) {
    size_t k = std::stoul(key);
    unsigned dbid = k%ndb;
    if (db[dbid]->check(key) == -1) {
        return false;
    }
    return true;
}

bool MultiKCDatabase::get_record(std::string key, std::string* value) {
    size_t k = std::stoul(key);
    unsigned dbid = k%ndb;
    if (!db[dbid]->get(key, value)) {
        //std::cerr << "get error: " << db[dbid]->error().name() << std::endl;
        return false;
    }
    return true;
}

MultiKCDatabase::MultiKCIterator::MultiKCIterator(MultiKCDatabase* db, std::vector<kyotocabinet::PolyDB::Cursor*>* cursors) {
    this->current_db = 0;
    this->db = db;
    this->cursors = cursors;
    this->current_key = "";
    this->current_value = "";
    this->reset();
}

void MultiKCDatabase::MultiKCIterator::reset() {
    for (auto c: *cursors) {
        c->jump();
    }
    this->current_db = 0;
    this->next();
}

bool MultiKCDatabase::MultiKCIterator::has_next() {
    return hn;
}

std::pair<std::string, std::string> MultiKCDatabase::MultiKCIterator::next() {
    std::pair<std::string, std::string> cur_val = std::make_pair(current_key, current_value);
    std::string ckey, cvalue;
    bool status = this->cursors->at(current_db)->get(&ckey, &cvalue, true);
    if (status) {
        current_key = ckey;
        current_value = cvalue;
        this->hn = status;
    } else if (current_db+1 < cursors->size()){
        current_db++;
        this->next();
    } else {
        this->hn = status;
    }    
    return cur_val;
}

MultiKCDatabase::MultiKCIterator* MultiKCDatabase::get_iterator() {
    return it;
}