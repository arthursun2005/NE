//
//  ne.cpp
//  NE
//
//  Created by Arthur Sun on 8/24/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "population.h"

void ne_genome::flush() {
    ne_uint size = nodes.size();
    
    for(ne_uint i = settings->input_size; i != size; ++i) {
        nodes[i]->value = 0.5;
    }
}

void ne_genome::activate() {
    size_t size = nodes.size();
    
    for(size_t i = settings->input_size; i != size; ++i) {
        ne_float sum = 0.0;
        
        for(ne_link* link : nodes[i]->links)
            if(link->enabled) sum += link->weight * link->i->value;
        
        nodes[i]->value = 1.0 / (1.0 + exp(-sum));
    }
}

void ne_genome::mutate_add_node() {    
    ne_link* link = links[ne_random(0, links.size() - 1)];
    
    if(!link->enabled || link->i == 0) return;
    
    ne_node* node = new ne_node();
    nodes.insert(nodes.end() - settings->output_size, node);
    
    ne_link* link1 = new ne_link(link->i, node);
    link1->enabled = true;
    link1->weight = 1.0;
    add(link1);
    
    ne_link* link2 = new ne_link(node, link->j);
    link2->enabled = true;
    link2->weight = link->weight;
    add(link2);
    
    link->enabled = false;
}

void ne_genome::mutate_add_link() {
    ne_uint size = nodes.size() - 1;
    
    ne_link q(nodes[ne_random(0, size - settings->output_size)], nodes[ne_random(settings->input_size, size)]);
    
    ne_link_set::iterator it = link_set.find(&q);
    if(it != link_set.end()) {
        if(!(*it)->enabled) {
            (*it)->enabled = true;
            (*it)->weight = ne_random(-2.0, 2.0);
        }
    }else{
        ne_link* link = new ne_link(q);
        link->enabled = true;
        link->weight = ne_random(-2.0, 2.0);
        add(link);
    }
}

ne_genome* ne_population::analyse() {
    average_fitness = 0.0;
    
    for(ne_genome* g : genomes) {
        g->fitness = fmax(0.0, g->fitness);
        average_fitness += g->fitness;
    }
    
    average_fitness /= (ne_float) settings.population;
    
    std::sort(genomes.begin(), genomes.end(), [] (ne_genome* a, ne_genome* b) {
        return a->fitness > b->fitness;
    });
    
    return genomes[0];
}

void ne_population::reproduce() {
    std::vector<ne_genome*> babies;
    
    if(average_fitness != 0.0) {        
        for(ne_genome* g : genomes) {
            ne_uint offsprings = (ne_uint)floor(g->fitness / average_fitness);
            
            for(ne_uint n = 0; n != offsprings; ++n)
                babies.push_back(breed(g));
        }
    }
    
    ne_uint leftover = settings.population - babies.size();
    for(ne_genome* g : genomes) {
        if(leftover == 0) break;
        babies.push_back(breed(g));
        --leftover;
    }
    
    for(ne_genome* g : genomes)
        delete g;
    
    genomes = babies;
}
