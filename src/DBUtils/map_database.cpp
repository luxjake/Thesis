#include <iostream>

#include "map_database.hpp"

MapDatabase::MapDatabase() {
    this->mapdb = new std::unordered_map<size_t, std::string>();
    this->it = new MapDbIterator(this->mapdb);
}

MapDatabase::~MapDatabase() {
    delete this->mapdb;
    delete this->it;
}

bool MapDatabase::exist(std::string key) {
    size_t k = std::stoul(key);
    size_t count = this->mapdb->count(k); 
    if (count == 0) {
        return false;
    }
    return true;
}

std::pair<std::string, bool> MapDatabase::add_record(std::string key, std::string value) {
    size_t k = std::stoul(key);
    std::pair<std::string, bool> p;
    auto ri = this->mapdb->emplace(std::make_pair(k, value));
    p = std::make_pair((*(ri.first)).second, ri.second);
    return p;
}

void MapDatabase::fast_add_record(std::string key, std::string value) {
    this->add_record(key, value);
}

bool MapDatabase::remove_record(std::string key) {
    size_t k = std::stoul(key);
    size_t er = this->mapdb->erase(k);
    if (er > 0) {
        return true;
    }
    return false;
}

bool MapDatabase::get_record(std::string key, std::string* value) {
    if (exist(key)) {
        size_t k = std::stoul(key);
        *value = this->mapdb->at(k);
        return true;
    }    
    return false;
}

MapDatabase::MapDbIterator* MapDatabase::get_iterator() {
    return this->it;
}

MapDatabase::MapDbIterator::MapDbIterator(std::unordered_map<size_t, std::string>* db) {
    this->db = db;
    this->it = this->db->begin();
}

bool MapDatabase::MapDbIterator::has_next() {
    if (this->it == this->db->end()) {
        return false;
    }
    return true;
}

void MapDatabase::MapDbIterator::reset() {
    this->it = this->db->begin();
}

std::pair<std::string, std::string> MapDatabase::MapDbIterator::next() {
    std::pair<std::string, std::string> p = std::make_pair(std::to_string(this->it->first), this->it->second);
    this->it++;
    return p;
}
