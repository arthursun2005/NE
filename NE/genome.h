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

struct ne_genome
{
    ne_float fitness;
    
    std::vector<ne_node*> nodes;
    std::vector<ne_link*> links;
    
    ne_link_set link_set;
    
    ne_uint input_size;
    ne_uint output_size;
    
    inline ne_genome(const ne_genome& genome) {
        *this = genome;
    }
    
    ne_genome(const ne_params& params);
    
    ne_genome& operator = (const ne_genome& genome);
    
    inline ~ne_genome() {
        for(ne_link* link : links)
            delete link;
        
        for(ne_node* node : nodes)
            delete node;
    }
    
    inline ne_node** inputs() {
        return nodes.data();
    }
    
    inline ne_node** outputs() {
        return nodes.data() + nodes.size() - output_size;
    }
    
    void flush();
    void activate();
    
    void mutate_add_node(ne_params& params);
    void mutate_add_link();
    void mutate_weights(const ne_params& params);
    
    inline void insert(ne_node* node) {
        std::vector<ne_node*>::iterator end = nodes.end();
        std::vector<ne_node*>::iterator begin = nodes.begin();
        
        while(end-- != begin) {
            if(node->id > (*end)->id) break;
        }
        
        ++end;
        nodes.insert(end, node);
    }
    
    inline void insert(ne_link* link) {
        links.push_back(link);
        link_set.insert(link);
        
        link->j->links.push_back(link);
    }
    
};

void ne_mutate(ne_genome* g, ne_params& params);

#endif /* genome_h */
