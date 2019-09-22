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
        
    inline ne_genome(const ne_genome& genome) {
        settings = genome.settings;
        
        for(ne_node* node : genome.nodes) {
            ne_node* clone = new ne_node();
            node->clone = clone;
            nodes.push_back(clone);
        }
        
        for(ne_link* link : genome.links) {
            if(!link->enabled) continue;
            ne_link* clone = new ne_link(link->i->clone, link->j->clone);
            clone->enabled = true;
            clone->weight = link->weight;
            add(clone);
        }
    }
    
    inline ne_genome(ne_settings* settings) : settings(settings) {
        size_t q = settings->input_size + settings->output_size;
        
        for(size_t i = 0; i != q; ++i)
            nodes.push_back(new ne_node());
    }
    
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
    
    inline void adapt(double rate) {
        for(ne_link* link : links)
            if(link->enabled) link->weight += 2.0 * rate * (link->i->value - 0.5) * link->weight;
    }
    
    inline void mutate_weight() {
        links[ne_random(0, links.size() - 1)]->weight += ne_random(-1.0, 1.0);
    }
    
    inline void add(ne_link* link) {
        links.push_back(link);
        link_set.insert(link);
        link->j->links.push_back(link);
    }
    
    inline void mutate() {
        mutate_weight();
        
        if(ne_random(0.0, 1.0) < settings->mutate_add_node_prob)
            mutate_add_node();
        
        if(ne_random(0.0, 1.0) < settings->mutate_add_link_prob)
            mutate_add_link();
    }
    
    inline void write(std::ofstream& os) const {
        size_t q = nodes.size();
        os.write((char*)&q, sizeof(q));
        for(ne_link* link : links) {
            os.write((char*)&link->i, sizeof(q));
        }
    }
    
};

#endif /* genome_h */
