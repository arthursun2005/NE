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
    
    ne_settings* settings;
    
    inline ne_genome() {}
    
    ne_genome(const ne_genome& genome);
    
    ne_genome(ne_settings* _settings);
    
    ne_genome& operator = (const ne_genome& genome) = delete;
    
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
        return nodes.data() + nodes.size() - settings->output_size;
    }
    
    void flush();
    void activate();
    
    void mutate_add_node();
    void mutate_add_link();
    
    inline void mutate_weight() {
        links[ne_random(0, links.size() - 1)]->weight += ne_random(-1.0, 1.0);
    }
    
    static ne_genome* crossover(ne_genome* a, ne_genome* b);
    
    inline void add(ne_link* link) {
        links.push_back(link);
        link_set.insert(link);
        
        link->j->links.push_back(link);
    }
    
};

#endif /* genome_h */
