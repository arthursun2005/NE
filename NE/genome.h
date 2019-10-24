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

struct ne_genome {
    double fitness;
    
    std::vector<ne_node*> nodes;
    std::vector<ne_link*> links;
    
    ne_link_set link_set;
    
    size_t input_size;
    size_t output_size;
    
    ne_genome(const ne_genome& genome) : input_size(genome.input_size), output_size(genome.output_size) {
        size_t size = genome.nodes.size();
        nodes.resize(size);
        for(size_t i = 0; i != size; ++i) {
            nodes[i] = new ne_node();
            genome.nodes[i]->clone = i;
        }
        
        for(ne_link* link : genome.links) {
            if(link->weight == 0.0) continue;
            ne_link* clone = new ne_link(nodes[link->i->clone], nodes[link->j->clone]);
            clone->weight = link->weight;
            add(clone);
        }
    }
    
    ne_genome(size_t input_size, size_t output_size) : input_size(input_size), output_size(output_size) {
        nodes.resize(input_size + output_size);
        
        for(ne_node*& node : nodes)
            node = new ne_node();
    }
    
    ne_genome(std::ifstream& is) {
        size_t q;
        is.read((char*)&q, sizeof(q));
        nodes.resize(q);
        is.read((char*)&q, sizeof(q));
        links.resize(q);
        size_t i, j;
        for(ne_link* link : links) {
            is.read((char*)&i, sizeof(i));
            is.read((char*)&j, sizeof(j));
            is.read((char*)&link->weight, sizeof(link->weight));
            
            link->i = nodes[i];
            link->j = nodes[j];
            
            link_set.insert(link);
            link->j->links.push_back(link);
        }
    }
    
    ne_genome& operator = (const ne_genome& genome) = delete;
    
    ~ne_genome() {
        for(ne_link* link : links)
            delete link;
        
        for(ne_node* node : nodes)
            delete node;
    }
    
    ne_node** inputs() {
        return nodes.data();
    }
    
    ne_node** outputs() {
        return nodes.data() + nodes.size() - output_size;
    }
    
    void flush() {
        size_t size = nodes.size();
        
        for(size_t i = input_size; i != size; ++i) {
            nodes[i]->value = 0.0;
        }
    }
    
    void activate() {
        size_t size = nodes.size();
        
        for(size_t i = input_size; i != size; ++i) {
            double sum = 0.0;
            
            for(ne_link* link : nodes[i]->links)
                sum += link->weight * link->i->value;
            
            nodes[i]->value = tanh(sum);
        }
    }
    
    void mutate_add_node() {
        ne_link* link = links[ne_random(0lu, links.size() - 1)];
        
        if(link->weight == 0.0 || link->i == 0) return;
        
        ne_node* node = new ne_node();
        nodes.insert(nodes.end() - output_size, node);
        
        ne_link* link1 = new ne_link(link->i, node);
        link1->weight = 1.0;
        add(link1);
        
        ne_link* link2 = new ne_link(node, link->j);
        link2->weight = link->weight;
        add(link2);
        
        link->weight = 0.0;
    }
    
    void mutate_add_link() {
        size_t size = nodes.size() - 1;
        
        ne_link q(nodes[ne_random(0lu, size - output_size)], nodes[ne_random(input_size, size)]);
        
        ne_link_set::iterator it = link_set.find(&q);
        if(it != link_set.end()) {
            if((*it)->weight == 0.0) {
                (*it)->weight = ne_random(-2.0, 2.0);
            }
        }else{
            ne_link* link = new ne_link(q);
            link->weight = ne_random(-2.0, 2.0);
            add(link);
        }
    }
    
    void adapt(double rate) {
        for(ne_link* link : links)
            link->weight += rate * link->i->value * link->j->value;
    }
    
    void mutate_weight() {
        links[ne_random(0lu, links.size() - 1)]->weight += ne_random(-2.0, 2.0);
    }
    
    void add(ne_link* link) {
        links.push_back(link);
        link_set.insert(link);
        link->j->links.push_back(link);
    }
    
    void mutate(double mutate_add_prob) {
        mutate_weight();
        
        if(ne_random(0.0, 1.0) < mutate_add_prob)
            mutate_add_node();
        
        mutate_add_link();
    }
    
    void write(std::ofstream& os) const {
        size_t q;
        q = nodes.size();
        os.write((char*)&q, sizeof(q));
        
        for(size_t i = 0; i < q; ++i)
            nodes[i]->clone = i;
        
        q = links.size();
        os.write((char*)&q, sizeof(q));
        
        for(ne_link* link : links) {
            os.write((char*)&link->i->clone, sizeof(link->i->clone));
            os.write((char*)&link->j->clone, sizeof(link->j->clone));
            os.write((char*)&link->weight, sizeof(link->weight));
        }
    }
};

#endif /* genome_h */
