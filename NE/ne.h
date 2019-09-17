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
#include <iostream>

typedef float float32;
typedef double float64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

#define ne_function(x) (1.0 / (1.0 + exp(-x)))

#define ne_half_nodes (1llu << 63)

#define ne_hash(i, j) ((((i + j) * (i + j + 1)) >> 1) + j)

typedef float64 ne_float;
typedef uint64 ne_uint;
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
    
    bool active;
    bool activated;
    ne_uint id;
    
    ne_node* clone;
    std::vector<ne_link*> links;
    
    ne_node(ne_uint id) : id(id) {}
};

struct ne_link
{
    ne_node* i;
    ne_node* j;

    ne_float weight;
    
    ne_link(ne_node* i, ne_node* j) : i(i), j(j) {}
    
    inline bool enabled() const {
        return weight != 0.0;
    }
    
    inline void disable() {
        weight = 0.0;
    }
};

struct ne_link_hash
{
    inline ne_uint operator () (const ne_link* x) const {
        return ne_hash(x->i->id, x->j->id);
    }
};

struct ne_link_equal
{
    inline bool operator () (const ne_link* a, const ne_link* b) const {
        return a->i->id == b->i->id && a->j->id == b->j->id;
    }
};

typedef std::unordered_set<ne_link*, ne_link_hash, ne_link_equal> ne_link_set;

struct ne_settings
{
    ne_float mutate_add_node_prob;
    ne_float mutate_add_link_prob;
    
    ne_uint ids;
    ne_uint population;
    
    ne_uint input_size;
    ne_uint output_size;
    
    ne_settings() {
        mutate_add_node_prob = 0.01;
        mutate_add_link_prob = 0.2;
        
        population = 256;
    }
};


#endif /* ne_h */
