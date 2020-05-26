#include <iostream>

#include "rocks_database.hpp"

RocksDatabase::RocksDatabase(std::string path) {
    rocksdb::Options options;
    options.create_if_missing = true;
    options.max_open_files = -1;
    options.compression = rocksdb::kLZ4Compression;
    rocksdb::Status status = rocksdb::DB::Open(options, path, &this->db);
    assert(status.ok());
    this->it = new RocksDbIterator(this->db);
}

RocksDatabase::~RocksDatabase() {
    delete this->db;
    delete this->it;
}

std::pair<std::string, bool> RocksDatabase::add_record(std::string key, std::string value) {
    rocksdb::Status s = db->Put(rocksdb::WriteOptions(), key, value);
    return std::make_pair(value, s.ok());
}

void RocksDatabase::fast_add_record(std::string key, std::string value) {
    db->Put(rocksdb::WriteOptions(), key, value);
}

bool RocksDatabase::remove_record(std::string key) {
    rocksdb::Status s = db->Delete(rocksdb::WriteOptions(), key);
    return s.ok();
}

bool RocksDatabase::get_record(std::string key, std::string* value) {
    rocksdb::Status s = db->Get(rocksdb::ReadOptions(), key, value);
    return s.ok();
}

bool RocksDatabase::exist(std::string key) {
    return get_record(key, nullptr);
}

RocksDatabase::RocksDbIterator* RocksDatabase::get_iterator() {
    return this->it;
}

RocksDatabase::RocksDbIterator::RocksDbIterator(rocksdb::DB* db) {
    this->db = db;
    this->it = nullptr;
}

void RocksDatabase::RocksDbIterator::reset() {
    if (this->it) {
        delete this->it;
    }
    uint64_t nkey;
    this->db->GetAggregatedIntProperty("rocksdb.estimate-num-keys", &nkey);
    this->it = this->db->NewIterator(rocksdb::ReadOptions());
    if (nkey > 0 && this->it->status().ok()) {
        this->it->SeekToFirst();
    }
}

bool RocksDatabase::RocksDbIterator::has_next() {
    if (this->it) {
        return this->it->Valid();
    } else {
        return false;
    }    
}

std::pair<std::string, std::string> RocksDatabase::RocksDbIterator::next() {
    if (!this->it) {
        this->reset();
    }
    auto p = std::make_pair(this->it->key().ToString(), this->it->value().ToString());
    this->it->Next();
    if (!this->has_next()) {
        delete this->it;
        this->it = nullptr;
    }
    return p;
}