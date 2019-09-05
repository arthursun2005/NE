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
#include <fstream>
#include <unordered_set>

#define ne_reduce(x) (0.5 + 0.5 * x / (1.0 + fabs(x)))

#define ne_accuracy 16

#define ne_bytes (sizeof(float64) * ne_accuracy)

struct ne_params
{
    float64 mutate_function_prob;
    float64 mutate_function_rate;
    float64 mutate_function_power;
    
    float64 mutate_weight_prob;
    float64 mutate_weight_rate;
    float64 mutate_weight_power;
    
    float64 mutate_add_node_prob;
    float64 mutate_add_gene_prob;
    
    float64 mutate_activation_prob;
    
    float64 mutate_only_prob;
    float64 mate_only_prob;
    float64 mate_avg_prob;
    
    float64 survive_thresh;
    
    float64 compat_node;
    float64 compat_gene;
    float64 compat_function;
    float64 compat_weight;
    float64 compat_thresh;
    
    uint64 node_ids;
    uint64 gene_ids;
    uint64 input_size;
    uint64 output_size;
    uint64 population;
    
    ne_params() {
        mutate_function_prob = 0.75;
        mutate_function_rate = 0.75;
        mutate_function_power = 2.0;
        
        mutate_weight_prob = 0.75;
        mutate_weight_rate = 0.75;
        mutate_weight_power = 2.0;
        
        mutate_add_node_prob = 0.03;
        mutate_add_gene_prob = 0.05;
        
        mutate_activation_prob = 0.5;
        
        mutate_only_prob = 0.25;
        mate_only_prob = 0.25;
        mate_avg_prob = 0.4;
        
        survive_thresh = 0.4;
        
        compat_node = 2.0;
        compat_gene = 1.0;
        compat_function = 1.0;
        compat_weight = 1.0;
        compat_thresh = 5.0;
        
        node_ids = 0;
        gene_ids = 0;
        population = 256;
    }
};

class ne_function
{
    
protected:
    
    float64 values[ne_accuracy];
    
public:
    
    ne_function() {}
    
    ne_function(const ne_function& function) {
        std::memcpy(values, function.values, ne_bytes);
    }
    
    ne_function& operator = (const ne_function& function) {
        std::memcpy(values, function.values, ne_bytes);
        return *this;
    }
    
    static void crossover(const ne_function* a, const ne_function* b, ne_function* baby) {
        for(uint64 i = 0; i < ne_accuracy; ++i) {
            baby->values[i] = 0.5 * (a->values[i] + b->values[i]);
        }
    }
    
    static float64 distance(const ne_function* a, const ne_function* b) {
        float64 d = 0.0;
        float64 q;
        
        for(uint64 i = 0; i < ne_accuracy; ++i) {
            q = b->values[i] - a->values[i];
            d += q * q;
        }
        
        return sqrt(d / (float64) ne_accuracy);
    }
    
    inline float64 operator () (float64 x) const {
        x = ne_reduce(x) * ne_accuracy;
        uint64 i = (uint64) x;
        uint64 j = i + 1;
        return values[i] * ((float64) j - x) + values[j] * (x - (float64) i);
    }
    
    inline void randomlize() {
        for(uint64 i = 0; i < ne_accuracy; ++i) {
            values[i] = gaussian_random();
        }
    }
    
    inline void mutate(const ne_params& params) {
        for(uint64 i = 0; i < ne_accuracy; ++i) {
            values[i] += random(-params.mutate_function_power, params.mutate_function_power);
        }
    }
    
    inline void write(std::ofstream& os) const {
        os.write((char*)values, ne_bytes);
    }
    
    inline void read(std::ifstream& is) {
        is.read((char*)values, ne_bytes);
    }
    
};

struct ne_node
{
    float64 value;
    float64 sum;
    
    bool computed;
    bool activated;
    
    uint64 id;
    
    ne_function function;
    
    ne_node(uint64 id) : id(id) {
        function.randomlize();
    }
    
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
