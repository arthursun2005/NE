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

typedef double ne_float;
typedef unsigned long ne_uint;
typedef std::mt19937_64 ne_generator_type;

static ne_generator_type ne_generator = ne_generator_type(std::chrono::high_resolution_clock::now().time_since_epoch().count());

inline ne_uint ne_random() {
    return ne_generator();
}

inline ne_float ne_random(ne_float a, ne_float b) {
    return std::uniform_real_distribution<ne_float>(a, b)(ne_generator);
}

inline ne_uint ne_random(ne_uint a, ne_uint b) {
    return std::uniform_int_distribution<ne_uint>(a, b)(ne_generator);
}

struct ne_link;

struct ne_node
{
    ne_float value;
    ne_uint clone;
    std::vector<ne_link*> links;
};

struct ne_link
{
    ne_node* i;
    ne_node* j;
    
    bool enabled;
    ne_float weight;
    
    ne_link(ne_node* i, ne_node* j) : i(i), j(j) {}
};

struct ne_link_hash
{
    inline size_t operator () (const ne_link* x) const {
        return (size_t)x->i ^ (size_t)x->j;
    }
};

struct ne_link_equal
{
    inline bool operator () (const ne_link* a, const ne_link* b) const {
        return a->i == b->i && a->j == b->j;
    }
};

typedef std::unordered_set<ne_link*, ne_link_hash, ne_link_equal> ne_link_set;

struct ne_settings
{
    ne_float mutate_add_node_prob;
    ne_float mutate_add_link_prob;
    
    ne_uint population;
    
    ne_uint input_size;
    ne_uint output_size;
    
    inline void read(std::ifstream& is) {
        is >> mutate_add_node_prob >> mutate_add_link_prob >> population;
    }
};


#endif /* ne_h */
