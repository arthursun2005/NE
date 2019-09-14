//
//  ne.h
//  NE
//
//  Created by Arthur Sun on 8/23/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef ne_h
#define ne_h

#include "common.h"
#include <vector>
#include <fstream>
#include <random>
#include <unordered_set>

#define ne_function(x) (1.0 / (1.0 + exp(-x)))

#define ne_half_nodes (1ull << 63)

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

struct ne_gene;

struct ne_node
{
    ne_float value;
    ne_float sum;
    
    bool activated;
    ne_uint id;
    
    std::vector<ne_gene*> genes;
    
    static inline bool sort (const ne_node* a, const ne_node* b) {
        return a->id < b->id;
    }
};

struct ne_node_hash
{
    inline ne_uint operator () (const ne_node* x) const {
        return x->id;
    }
};

struct ne_node_equal
{
    inline bool operator () (const ne_node* a, const ne_node* b) const {
        return a->id == b->id;
    }
};

struct ne_gene
{
    ne_node* i;
    ne_node* j;
    
    ne_uint id;
    ne_float weight;
    
    ne_gene() {}
    
    ne_gene(ne_node* i, ne_node* j) : i(i), j(j) {}
    
    static inline bool sort (const ne_gene* a, const ne_gene* b) {
        return a->id < b->id;
    }
    
    inline bool enabled() const {
        return weight != 0.0;
    }
    
    inline void disable() {
        weight = 0.0;
    }
};

inline size_t ne_hash(ne_uint i, ne_uint j)  {
    return (((i + j) * (i + j + 1)) >> 1) + j;
}

struct ne_gene_hash
{
    inline ne_uint operator () (const ne_gene* x) const {
        return ne_hash(x->i->id, x->j->id);
    }
};

struct ne_gene_equal
{
    inline bool operator () (const ne_gene* a, const ne_gene* b) const {
        return a->i->id == b->i->id && a->j->id == b->j->id;
    }
};

struct ne_innovation
{
    const ne_gene* gene;
    ne_uint node_id;
    ne_uint gene_id;
    bool new_node;
};

struct ne_innovation_hash
{
    inline ne_uint operator () (const ne_innovation& x) const {
        return ne_hash(x.gene->i->id, x.gene->j->id);
    }
};

struct ne_innovation_equal
{
    inline bool operator () (const ne_innovation& a, const ne_innovation& b) const {
        return a.new_node == b.new_node && a.gene->i->id == b.gene->i->id && a.gene->j->id == b.gene->j->id;
    }
};

typedef std::unordered_set<ne_node*, ne_node_hash, ne_node_equal> ne_node_set;
typedef std::unordered_set<ne_gene*, ne_gene_hash, ne_gene_equal> ne_gene_set;
typedef std::unordered_set<ne_innovation, ne_innovation_hash, ne_innovation_equal> ne_innovation_set;

struct ne_params
{
    ne_float mutate_weights_prob;
    ne_float mutate_weights_power;
    
    ne_float mutate_add_node_prob;
    ne_float mutate_add_gene_prob;
    
    ne_float mutate_only_prob;
    
    ne_float survive_thresh;
    
    ne_float compat_gene;
    ne_float compat_weight;
    ne_float compat_thresh;
    
    ne_uint node_ids;
    ne_uint gene_ids;
    ne_uint input_size;
    ne_uint output_size;
    ne_uint population;
    
    ne_innovation_set set;
    
    ne_params() {
        mutate_weights_prob = 0.8;
        mutate_weights_power = 2.0;
        
        mutate_add_node_prob = 0.01;
        mutate_add_gene_prob = 0.1;
        
        mutate_only_prob = 0.25;
        
        survive_thresh = 0.4;
        
        compat_gene = 1.0;
        compat_weight = 2.0;
        compat_thresh = 3.0;
        
        node_ids = 0;
        gene_ids = 0;
        population = 256;
    }
    
    ne_innovation add(const ne_gene* gene, bool new_node) {
        ne_innovation innovation;
        innovation.gene = gene;
        innovation.new_node = new_node;
        ne_innovation_set::iterator it = set.find(innovation);
        
        if(it != set.end()) {
            innovation.node_id = it->node_id;
            innovation.gene_id = it->gene_id;
        }else{
            if(new_node) {
                innovation.node_id = node_ids++;
                innovation.gene_id = gene_ids;
                gene_ids += 2;
            }else{
                innovation.gene_id = gene_ids++;
            }
            
            set.insert(innovation);
        }
        
        return innovation;
    }
};


#endif /* ne_h */
