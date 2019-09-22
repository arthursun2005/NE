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
        nodes.resize(genome.nodes.size());
        
        for(ne_node*& node : nodes)
            node = new ne_node();
        
        for(ne_link* link : genome.links) {
            if(!link->enabled) continue;
            ne_link* clone = new ne_link(link->i, link->j);
            clone->enabled = true;
            clone->weight = link->weight;
            add(clone);
        }
    }
    
    inline ne_genome(ne_settings* settings) : settings(settings) {
        nodes.resize(settings->input_size + settings->output_size);
        
        for(ne_node*& node : nodes)
            node = new ne_node();
    }
    
    inline ne_genome(std::ifstream& is) {
        size_t q;
        is.read((char*)&q, sizeof(q));
        nodes.resize(q);
        is.read((char*)&q, sizeof(q));
        links.resize(q);
        for(ne_link* link : links) {
            is.read((char*)&link->i, sizeof(link->i));
            is.read((char*)&link->j, sizeof(link->j));
            is.read((char*)&link->enabled, sizeof(link->enabled));
            is.read((char*)&link->weight, sizeof(link->weight));
            
            link_set.insert(link);
            nodes[link->j]->links.push_back(link);
        }
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
            if(link->enabled) link->weight += rate * (nodes[link->i]->value - 0.5) * (nodes[link->j]->value - 0.5);
    }
    
    inline void mutate_weight() {
        links[ne_random(0, links.size() - 1)]->weight += ne_random(-1.0, 1.0);
    }
    
    inline void add(ne_link* link) {
        links.push_back(link);
        link_set.insert(link);
        nodes[link->j]->links.push_back(link);
    }
    
    inline void mutate() {
        mutate_weight();
        
        if(ne_random(0.0, 1.0) < settings->mutate_add_node_prob)
            mutate_add_node();
        
        if(ne_random(0.0, 1.0) < settings->mutate_add_link_prob)
            mutate_add_link();
    }
    
    inline void write(std::ofstream& os) const {
        size_t q;
        q = nodes.size();
        os.write((char*)&q, sizeof(q));
        q = links.size();
        os.write((char*)&q, sizeof(q));
        for(ne_link* link : links) {
            os.write((char*)&link->i, sizeof(link->i));
            os.write((char*)&link->j, sizeof(link->j));
            os.write((char*)&link->enabled, sizeof(link->enabled));
            os.write((char*)&link->weight, sizeof(link->weight));
        }
    }
    
};

#endif /* genome_h */
