#include <kccompress.h>
#include "kyoto_database.hpp"

KCDatabase::KCDatabase(std::string path) {
    kyotocabinet::HashDB* db = new kyotocabinet::HashDB();
    kyotocabinet::Compressor* comp = new kyotocabinet::ZLIBCompressor<kyotocabinet::ZLIB::GZIP>;
    db->tune_options(kyotocabinet::HashDB::TCOMPRESS);
    db->tune_compressor(comp);
    if (!db->open(path, kyotocabinet::HashDB::OWRITER | kyotocabinet::HashDB::OCREATE)) {
        std::cerr << "open error: " << db->error().name() << std::endl;
        std::cerr << "file : " << path << std::endl;
        return;
    }

    this->comp = comp;
    this->db = db;
    this->it = new KCIterator(db->cursor());
}

KCDatabase::~KCDatabase() {
    if (!this->db->close()) {
        std::cerr << "close error: " << this->db->error().name() << std::endl;
    }
    delete this->it;
    delete this->db;
    delete this->comp;
}

std::pair<std::string, bool> KCDatabase::add_record(std::string key, std::string value) {
    if (!db->add(key, value)) {
        std::string val;
        get_record(key, &val);
        return std::make_pair(val, false);
    }
    return std::make_pair(value, true);
}

void KCDatabase::fast_add_record(std::string key, std::string value) {
    db->add(key, value);
}

bool KCDatabase::remove_record(std::string key) {
    if (!db->remove(key)) {
        std::cerr << "remove error: " << db->error().name() << std::endl;
        return false;
    }
    return true;
}

bool KCDatabase::exist(std::string key) {
    if (db->check(key) == -1) {
        return false;
    }
    return true;
}

bool KCDatabase::get_record(std::string key, std::string* value) {
    if (!db->get(key, value)) {
        //std::cerr << "get error: " << db->error().name() << std::endl;
        return false;
    }
    return true;
}

KCDatabase::KCIterator::KCIterator(kyotocabinet::HashDB::Cursor* cursor) {
    this->cursor = cursor;
    this->current_key = "";
    this->current_value = "";
    this->reset();
}

void KCDatabase::KCIterator::reset() {
    this->cursor->jump();
    this->next();
}

bool KCDatabase::KCIterator::has_next() {
    return hn;
}

std::pair<std::string, std::string> KCDatabase::KCIterator::next() {
    std::pair<std::string, std::string> cur_val = std::make_pair(current_key, current_value);
    std::string ckey, cvalue;
    bool status = this->cursor->get(&ckey, &cvalue, true);
    this->hn = status;
    if (status) {
        current_key = ckey;
        current_value = cvalue;
    }
    return cur_val;
}

KCDatabase::KCIterator* KCDatabase::get_iterator() {
    return it;
}
