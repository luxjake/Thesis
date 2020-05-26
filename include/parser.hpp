#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

#include "database.hpp"
#include "shared_dataset_info.hpp"

class Parser {

    public:

        virtual void read_file(std::string file) = 0;
        virtual u_int64_t get_triple_id() = 0;
        virtual u_int64_t get_number_of_triples() = 0;
        virtual ~Parser() {}

        static bool is_float_number(const std::string& string){
            std::string::const_iterator it = string.begin();
            bool decimal_point = false;
            unsigned min_size = 0;
            if (string.size()>0 && (string[0] == '-' || string[0] == '+')) {
                it++;
                min_size++;
            }
            while (it != string.end()) {
                if ((*it == '.') || (*it == ',')) {
                    if (!decimal_point) {
                        decimal_point = true;
                    } else {
                        break;
                    }
                } else if (!std::isdigit(*it) && ((*it!='f') || it+1 != string.end() || !decimal_point)) {
                    break;
                }
                ++it;
            }
            return string.size()>min_size && it == string.end();
        }

        static void increase_vector(std::vector<bool>* vec, unsigned new_size) {
            unsigned current_size = vec->size();
            if (new_size <= current_size) {
                return;
            }
            for (unsigned i=current_size; i<new_size; i++) {
                vec->push_back(false);
            }
        }

        static std::string get_delimiter(const std::string file_name) {
            std::vector<std::string>* s = split(file_name, ".");
            if (s->size() > 1) {
                std::string last = s->at(s->size()-1);
                if (last == "tsv") {
                    std::cout << "File type for " << file_name << " is 'TSV'" << std::endl;
                    return "\u0009"; // tab
                } else if ((last == "nt") || (last == "n3")) {
                    std::cout << "File type for " << file_name << " is 'N-TRIPLE'" << std::endl;
                    return " "; // space
                }
            }
            std::cerr << "Error when determining file type for " << file_name << std::endl;
            return "";
        }

        static std::vector<std::string>* split(const std::string& str, const std::string& delimiter) {
            std::vector<std::string>* tokens = new std::vector<std::string>;
            size_t prev = 0, pos = 0;
            
            do {
                pos = str.find(delimiter, prev);
                if (pos == std::string::npos) {
                    pos = str.length();
                } 
                std::string token = str.substr(prev, pos-prev);
                if (!token.empty()) {
                    tokens->push_back(token);
                }
                prev = pos + delimiter.length();
            } while (pos < str.length() && prev < str.length());
            
            return tokens;
        }

    static void parse_triple(const std::string triple, const std::string delimiter, std::string& s, std::string& p, std::string& o) {
        std::vector<std::string>* sv = Parser::split(triple, delimiter);

        if (sv->size() > 0) {
            if (sv->back() == ".") {
                sv->pop_back();
            }
        }

        size_t size = sv->size();

        uint start_ind = 0;
        if (size > 2) {
            if (size != 3) {
                if (size == 4) {
                    if (!Parser::is_float_number((*sv)[size-1])) {
                        start_ind++;
                    }
                } else {
                    if ((*sv)[2].at(0) == '"') { // last part of the triple is probably a literal
                        std::string tmp;
                        for (uint i=2; i<sv->size(); i++) {
                            tmp += sv->at(i) + " ";
                        }
                        sv->at(2) = tmp;
                    } else {
                        // ????
                    }
                    
                }
            }

            s = (*sv)[start_ind];
            p = (*sv)[start_ind+1];
            o = (*sv)[start_ind+2];
        } else {
            s = "";
            p = "";
            o = "";
        }

        delete sv;
    }

    protected:
        bool store_triples;
        SharedDatasetInfo* sdi;
        std::vector<bool>* bitvec;

        uint64_t number_of_triples;
        uint64_t triple_id;

};