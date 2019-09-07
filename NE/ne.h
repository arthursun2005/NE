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
#include <unordered_set>

#define ne_function(x) (1.0 / (1.0 + exp(-x)))

#define ne_max_nodes (1ull << 40)

#define ne_initial_genes 8

struct ne_params
{
    float64 mutate_weight_prob;
    float64 mutate_weight_power;
    
    float64 mutate_add_node_prob;
    float64 mutate_add_gene_prob;
    
    float64 mutate_only_prob;
    float64 mate_only_prob;
    
    float64 survive_thresh;
    
    float64 compat_gene;
    float64 compat_weight;
    float64 compat_thresh;
    
    uint64 node_ids;
    uint64 gene_ids;
    uint64 input_size;
    uint64 output_size;
    uint64 population;
    
    ne_params() {
        mutate_weight_prob = 0.75;
        mutate_weight_power = 2.0;
        
        mutate_add_node_prob = 0.05;
        mutate_add_gene_prob = 0.2;
        
        mutate_only_prob = 0.25;
        mate_only_prob = 0.25;
        
        survive_thresh = 0.4;
        
        compat_gene = 1.0;
        compat_weight = 2.0;
        compat_thresh = 3.0;
        
        node_ids = 0;
        gene_ids = 0;
        population = 256;
    }
};

struct ne_gene;

struct ne_node
{
    float64 value;
    float64 sum;
    
    bool activated;
    uint64 id;
    
    std::vector<ne_gene*> genes;
    
    static inline bool sort (const ne_node* a, const ne_node* b) {
        return a->id < b->id;
    }
};

struct ne_node_hash
{
    inline uint64 operator () (const ne_node* x) const {
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
    
    uint64 id;
    float64 weight;
    
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

struct ne_gene_hash
{
    inline uint64 operator () (const ne_gene* x) const {
        return (((x->i->id + x->j->id) * (x->i->id + x->j->id + 1)) >> 1) + x->j->id;
    }
};

struct ne_gene_equal
{
    inline bool operator () (const ne_gene* a, const ne_gene* b) const {
        return a->i->id == b->i->id && a->j->id == b->j->id;
    }
};

typedef std::unordered_set<ne_node*, ne_node_hash, ne_node_equal> ne_node_set;
typedef std::unordered_set<ne_gene*, ne_gene_hash, ne_gene_equal> ne_gene_set;

#endif /* ne_h */
