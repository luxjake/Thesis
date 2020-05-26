#include <fstream>

#include "tsv_parser.hpp"

TSVParser::TSVParser(std::vector<bool>* bitvec, SharedDatasetInfo* sdi, bool store_triple) {
    this->store_triples = store_triple;
    this->bitvec = bitvec;
    this->sdi = sdi;
    triple_id = 0;
}

TSVParser::TSVParser(std::vector<bool>* bitvec, SharedDatasetInfo* sdi, bool store_triple, uint64_t id) {
    this->store_triples = store_triple;
    this->bitvec = bitvec;
    this->sdi = sdi;
    triple_id = id;
}

TSVParser::~TSVParser() {}

uint64_t TSVParser::get_triple_id() {
    return triple_id;
}

u_int64_t TSVParser::get_number_of_triples() {
    return number_of_triples;
}

void TSVParser::read_file(std::string file) {
    number_of_triples = 0;

    std::ifstream f(file, std::ifstream::in);
    if (f.fail()) {
        std::cerr << "The file " << file << " does not exist." << std::endl;
        return;
    }
    
    std::string delim = get_delimiter(file);
    std::hash<std::string> hash_fn;
    std::string l;
    while (std::getline(f, l)) {
        if (l.empty()) {
            continue;
        }
        
        std::string s,p,o;
        Parser::parse_triple(l, delim, s, p, o);

        if (!(s.empty()||p.empty()||o.empty())) {
            std::string triple = s + " " + p + " " + o;
            size_t hash = hash_fn(triple);
            std::string id = std::to_string(hash);
            
            uint64_t tid = sdi->get_identifier(id);
            if (tid+1 > bitvec->size()) {
                increase_vector(bitvec, tid+1);
            }
            bitvec->at(tid) = true;

            if (store_triples) {
                sdi->add_triple(id, l);
            }         
            
            number_of_triples++;
        }

    }

    f.close();
    
}

