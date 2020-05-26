#include <fstream>
#include <vector>

#include <omp.h>

#include "para_parser.hpp"

ParallelParser::ParallelParser(std::vector<bool>* bitvec, SharedDatasetInfo* sdi, bool store_triples) : stop(false), processed_lines(0) {
    this->bitvec = bitvec;
    this->sdi = sdi;
    this->store_triples = store_triples;
    triple_id = 0;
}

ParallelParser::~ParallelParser() {}

uint64_t ParallelParser::get_triple_id() {
    return triple_id;
}


uint64_t ParallelParser::get_number_of_triples() {
    return number_of_triples;
}


std::string ParallelParser::parse_line(std::string line) {
    std::vector<std::string>* sv = split(line, delimiter);
    std::string pl;
    
    if (sv->size() > 0) {
        if (sv->back() == ".") {
            sv->pop_back();
        }
    }

    int size = sv->size();

    int start_ind = 0;
    if (size > 1) {
        if (size != 3) {
            if (size == 4) {
                if (!is_float_number((*sv)[size-1])) {
                    start_ind++;
                }
            } else {
                start_ind++;
            }
        }
        
        std::string triple = "";
        
        for (unsigned i=start_ind; i<sv->size(); i++) {
            std::string tmp = (*sv)[i] + " ";
            triple.append(tmp);
        }
        pl = triple;
    } else {
        pl = "";
    }

    delete sv;
    return pl;
}

void ParallelParser::parsing_task() {
    std::hash<std::string> hash_fn;

    std::string line;
    std::string triple;

    while(1) {
        {
            std::unique_lock<std::mutex> ul(lock);

            while (!stop && to_process.empty()) {
                cond_var.wait(ul);
            }

            if (to_process.empty()) {
                return;
            }

            line = std::move(to_process.front());
            to_process.pop();

        }

        triple = parse_line(line);
        size_t hash = hash_fn(triple);
        std::string id = std::to_string(hash);
        uint64_t tid = sdi->get_identifier(id); //already thread safe

        {
            std::unique_lock <std::mutex> ul(bitvec_lock);
            if (tid+1 > bitvec->size()) {
                increase_vector(bitvec, tid+1);
            }
            bitvec->at(tid) = true;
        }

        if (store_triples) {
            sdi->add_triple(id, triple); //already thread safe
        }
    }
    
}

void ParallelParser::reading_task(std::string file) {
    number_of_triples = 0;
    std::ifstream f(file, std::ifstream::in);
    if (f.fail()) {
        std::cerr << "The file " << file << " does not exist." << std::endl;
        return;
    }

    std::string l;
    while (std::getline(f, l)) {
        if (l.empty()) {
            break;
        }
        {
            std::unique_lock <std::mutex> ul(lock);
            to_process.emplace(l);
            cond_var.notify_one();
        }
        number_of_triples++;
    }

    stop = true;
    f.close();
}


void ParallelParser::read_file(std::string file) {
    number_of_triples = 0;

    delimiter = get_delimiter(file);
    
    uint nt = store_triples ? 3 : 2;
    #pragma omp parallel num_threads(nt)
    {
        int tid = omp_get_thread_num();
        if (tid == 0) { // master thread, will read the file
            reading_task(file);
        } else { // worker threads, will parse each line
            parsing_task();
        }
    }
}