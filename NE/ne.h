//
//  ne.h
//  NE
//
//  Created by Arthur Sun on 8/23/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef ne_h
#define ne_h

#include <vector>
#include <fstream>
#include <random>
#include <unordered_set>
#include <cmath>
#include <functional>
#include <cfloat>

typedef std::mt19937_64 ne_generator_type;

static ne_generator_type ne_generator = ne_generator_type(std::chrono::high_resolution_clock::now().time_since_epoch().count());

template <class T>
inline T ne_random(T a, T b) {
    if(std::is_integral<T>::value) {
        return std::uniform_int_distribution<T>(a, b)(ne_generator);
    }
    
    return std::uniform_real_distribution<T>(a, b)(ne_generator);
}

struct ne_link;

struct ne_node {
    double value;
    size_t clone;
    std::vector<ne_link*> links;
};

struct ne_link {
    ne_node* i;
    ne_node* j;
    
    double weight;
    
    ne_link(ne_node* i, ne_node* j) : i(i), j(j) {}
};

struct ne_link_hash {
    size_t operator () (const ne_link* x) const {
        return (size_t)x->i ^ (size_t)x->j;
    }
};

struct ne_link_equal {
    bool operator () (const ne_link* a, const ne_link* b) const {
        return a->i == b->i && a->j == b->j;
    }
};

typedef std::unordered_set<ne_link*, ne_link_hash, ne_link_equal> ne_link_set;

#endif /* ne_h */
