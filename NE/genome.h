//
//  genome.h
//  NE
//
//  Created by Arthur Sun on 8/24/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef genome_h
#define genome_h

#include "ne.h"
#include <vector>

class ne_genome
{
    
public:
    
    ne_genome() {}
    
    ne_genome(const ne_genome& genome) {
        *this = genome;
    }
    
    ne_genome& operator = (const ne_genome& genome);
    
    ~ne_genome() {
        clear();
    }
    
    void reset(ne_params& params) {
        input_size = params.input_size + 1;
        output_size = params.output_size;
        clear();
        initialize();
        mutate_add_gene(params);
    }
    
    inline ne_node** inputs() {
        return nodes.data();
    }
    
    inline ne_node** outputs() {
        return nodes.data() + nodes.size() - output_size;
    }
    
    void flush() {
        uint64 size = nodes.size();
        
        for(uint64 i = 0; i < input_size; ++i) {
            nodes[i]->activated = true;
        }
        
        nodes[input_size - 1]->value = 1.0;
        
        for(uint64 i = input_size; i < size; ++i) {
            nodes[i]->activated = false;
        }
    }
    
    void compute();
    
    void mutate_add_node(ne_params& params);
    void mutate_add_gene(ne_params& params);
    void mutate_weights(const ne_params& params);
    
    static inline bool sort(const ne_genome* a, const ne_genome* b) {
        return a->fitness > b->fitness;
    }
    
    static ne_genome* crossover(const ne_genome* a, const ne_genome* b, ne_params& params);
    static float64 distance(const ne_genome* a, const ne_genome* b, const ne_params &params);
    
    float64 fitness;
    bool eliminated;
    
    std::vector<ne_node*> nodes;
    std::vector<ne_gene*> genes;
    
protected:
    
    inline void initialize() {
        for(uint64 i = 0; i < input_size; ++i) {
            ne_node node;
            node.id = i;
            find(&node);
        }
        
        for(uint64 i = 0; i < output_size; ++i) {
            ne_node node;
            node.id = i + ne_max_nodes;
            find(&node);
        }
    }
    
    inline void insert(ne_node* node) {
        insert_in_order(&nodes, node, ne_node::sort);
        node_set.insert(node);
    }
    
    inline void insert(ne_gene* gene) {
        insert_in_order(&genes, gene, ne_gene::sort);
        gene_set.insert(gene);
        
        gene->j->genes.push_back(gene);
    }
    
    inline ne_node* find(ne_node* node) {
        ne_node_set::iterator it = node_set.find(node);
        
        if(it == node_set.end()) {
            ne_node* new_node = new ne_node(*node);
            new_node->genes.clear();
            insert(new_node);
            return new_node;
        }else{
            return *it;
        }
    }
    
    inline void pass_down(ne_gene* gene) {
        gene->i = find(gene->i);
        gene->j = find(gene->j);
        
        insert(gene);
    }
    
    inline void clear() {
        node_set.clear();
        gene_set.clear();
        
        for(ne_gene* gene : genes)
            delete gene;
        
        for(ne_node* node : nodes)
            delete node;
        
        genes.clear();
        nodes.clear();
    }
    
    ne_node_set node_set;
    ne_gene_set gene_set;
    
    uint64 input_size;
    uint64 output_size;
    
};

inline void ne_mutate(ne_genome* g, ne_params& params) {
    if(random(0.0, 1.0) < params.mutate_add_node_prob) {
        g->mutate_add_node(params);
    }
    
    if(random(0.0, 1.0) < params.mutate_add_gene_prob) {
        g->mutate_add_gene(params);
    }
    
    if(random(0.0, 1.0) < params.mutate_weight_prob) {
        g->mutate_weights(params);
    }
}

#endif /* genome_h */
