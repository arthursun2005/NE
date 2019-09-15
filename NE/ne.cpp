//
//  ne.cpp
//  NE
//
//  Created by Arthur Sun on 8/24/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "population.h"

ne_genome::ne_genome(const ne_params& params) {
    input_size = params.input_size;
    output_size = params.output_size;
    
    for(ne_uint i = 0; i < input_size; ++i) {
        insert(new ne_node(i));
    }
    
    for(ne_uint i = 0; i < output_size; ++i) {
        insert(new ne_node(i + ne_half_nodes));
    }
}

ne_genome& ne_genome::operator = (const ne_genome &genome) {
    input_size = genome.input_size;
    output_size = genome.output_size;
    
    for(ne_node* node : genome.nodes) {
        ne_node* clone = new ne_node(node->id);
        node->clone = clone;
        insert(clone);
    }
    
    for(ne_link* link : genome.links) {
        if(!link->enabled()) continue;
        ne_link* clone = new ne_link(link->i->clone, link->j->clone);
        clone->weight = link->weight;
        insert(clone);
    }
    
    return *this;
}

void ne_genome::flush() {
    ne_uint size = nodes.size();
    
    for(ne_uint i = 0; i < input_size; ++i) {
        nodes[i]->activated = true;
    }
    
    for(ne_uint i = input_size; i < size; ++i) {
        nodes[i]->activated = false;
    }
}

void ne_genome::activate() {
    for(ne_node* node : nodes) {
        if(node->id < input_size) continue;
        
        node->activated = false;
        node->sum = 0.0;
        
        for(ne_link* link : node->links) {
            if(link->i->activated && link->enabled()) {
                node->activated = true;
                node->sum += link->weight * link->i->value;
            }
        }
    }
    
    for(ne_node* node : nodes) {
        if(node->id >= input_size && node->activated) {
            node->value = ne_function(node->sum);
        }
    }
}

void ne_genome::mutate_add_node(ne_params &params) {
    ne_link* link = links[ne_random() % links.size()];
    
    if(!link->enabled() || link->i->id == 0) return;
    
    ne_node* node = new ne_node(params.ids++);
    insert(node);
    
    ne_link* link1 = new ne_link(link->i, node);
    link1->weight = 1.0;
    insert(link1);
    
    ne_link* link2 = new ne_link(node, link->j);
    link2->weight = link->weight;
    insert(link2);
    
    link->disable();
}

void ne_genome::mutate_add_link() {
    ne_uint size = nodes.size() - 1;
    
    ne_link q(nodes[ne_random(0, size - output_size)], nodes[ne_random(input_size, size)]);
    
    ne_link_set::iterator it = link_set.find(&q);
    if(it != link_set.end()) {
        if(!(*it)->enabled()) {
            (*it)->weight = ne_random(-2.0, 2.0);
        }
    }else{
        ne_link* link = new ne_link(q);
        link->weight = ne_random(-2.0, 2.0);
        insert(link);
    }
}

void ne_genome::mutate_weights(const ne_params &params) {
    for(ne_link* link : links) {
        if(link->enabled())
            link->weight += ne_random(-params.mutate_weights_power, params.mutate_weights_power);
    }
}

ne_population& ne_population::operator = (const ne_population& population) {
    params = population.params;
    
    for(ne_genome* g : genomes)
        delete g;
    
    genomes.resize(params.population);
    
    for(ne_uint i = 0; i < params.population; ++i) {
        genomes[i] = new ne_genome(*population.genomes[i]);
        genomes[i]->mutate_add_link();
    }
    
    return *this;
}

void ne_population::initialize() {
    for(ne_genome* g : genomes)
        delete g;
    
    genomes.resize(params.population);
    
    for(ne_uint i = 0; i < params.population; ++i) {
        genomes[i] = new ne_genome(params);
        genomes[i]->mutate_add_link();
    }
    
    params.ids = params.input_size;
}

ne_genome* ne_population::breed(ne_genome *g) {
    ne_genome* baby = new ne_genome(*g);
    
    if(ne_random(0.0, 1.0) < params.mutate_add_node_prob) {
        g->mutate_add_node(params);
    }
    
    if(ne_random(0.0, 1.0) < params.mutate_add_link_prob) {
        g->mutate_add_link();
    }
    
    g->mutate_weights(params);
    
    return baby;
}

ne_genome* ne_population::analyse() {
    total_fitness = 0.0;
    ne_genome* best = genomes.front();
    
    for(ne_genome* g : genomes) {
        if(g->fitness > best->fitness)
            best = g;
        
        total_fitness += fmax(0.0, g->fitness);
    }
    
    return best;
}

void ne_population::reproduce() {
    std::vector<ne_genome*> babies;
    
    if(total_fitness != 0.0) {
        for(ne_genome* g : genomes) {
            ne_uint offsprings = (ne_uint)floor(params.population * (g->fitness / total_fitness));
            
            for(ne_uint n = 0; n < offsprings; ++n)
                babies.push_back(breed(g));
        }
    }
    
    ne_uint leftover = params.population - babies.size();
    for(ne_genome* g : genomes) {
        if(leftover == 0) break;
        babies.push_back(breed(g));
        --leftover;
    }
    
    for(ne_genome* g : genomes)
        delete g;
    
    genomes = babies;
}
