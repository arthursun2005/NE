//
//  ne.cpp
//  NE
//
//  Created by Arthur Sun on 8/24/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "population.h"

ne_genome::ne_genome(ne_settings& _settings) {
    settings = &_settings;
    
    for(ne_uint i = 0; i != settings->input_size; ++i) {
        insert(new ne_node(i));
    }
    
    for(ne_uint i = 0; i != settings->output_size; ++i) {
        insert(new ne_node(i + ne_half_nodes));
    }
}

ne_genome::ne_genome(const ne_genome &genome) {
    settings = genome.settings;
    
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
}

void ne_genome::flush() {
    ne_uint size = nodes.size();
    
    for(ne_uint i = 0; i != settings->input_size; ++i) {
        nodes[i]->activated = true;
    }
    
    for(ne_uint i = settings->input_size; i != size; ++i) {
        nodes[i]->activated = false;
    }
}

void ne_genome::activate() {
    for(ne_node* node : nodes) {
        if(node->id < settings->input_size) continue;
        
        node->active = false;
        ne_float sum = 0.0;
        
        for(ne_link* link : node->links) {
            if(link->i->activated && link->enabled()) {
                node->active = true;
                sum += link->weight * link->i->value;
            }
        }
        
        node->value = ne_function(sum);
        
        if(node->active) {
            node->activated = true;
        }
    }
}

void ne_genome::mutate_add_node() {
    ne_link* link = links[ne_random() % links.size()];
    
    if(!link->enabled() || link->i->id == 0) return;
    
    ne_node* node = new ne_node(settings->ids++);
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
    
    ne_link q(nodes[ne_random(0, size - settings->output_size)], nodes[ne_random(settings->input_size, size)]);
    
    ne_link_set::iterator it = link_set.find(&q);
    if(it != link_set.end()) {
        if(!(*it)->enabled())
            (*it)->weight = ne_random(-2.0, 2.0);
    }else{
        ne_link* link = new ne_link(q);
        link->weight = ne_random(-2.0, 2.0);
        insert(link);
    }
}

void ne_genome::mutate_weight() {
    links[ne_random(0, links.size() - 1)]->weight += ne_random(-1.0, 1.0);
}

ne_population& ne_population::operator = (const ne_population& population) {
    settings = population.settings;
    
    clear();
    
    genomes.resize(settings.population);
    
    for(ne_uint i = 0; i != settings.population; ++i) {
        genomes[i] = new ne_genome(*population.genomes[i]);
    }
    
    return *this;
}

void ne_population::initialize() {
    clear();
    
    genomes.resize(settings.population);
    settings.ids = settings.input_size;
    
    for(ne_uint i = 0; i != settings.population; ++i) {
        genomes[i] = new ne_genome(settings);
        genomes[i]->mutate_add_link();
    }
}

ne_genome* ne_population::breed(ne_genome *g) {
    ne_genome* baby = new ne_genome(*g);
    
    if(ne_random(0.0, 1.0) < settings.mutate_add_node_prob) {
        baby->mutate_add_node();
    }
    
    if(ne_random(0.0, 1.0) < settings.mutate_add_link_prob) {
        baby->mutate_add_link();
    }
    
    baby->mutate_weight();
    
    return baby;
}

ne_genome* ne_population::analyse() {
    average_fitness = 0.0;
    
    for(ne_genome* g : genomes) {
        g->fitness = fmax(0.0, g->fitness);
        g->fitness *= g->fitness;
        average_fitness += g->fitness;
    }
    
    average_fitness /= (ne_float) settings.population;
    
    std::sort(genomes.begin(), genomes.end(), [] (ne_genome* a, ne_genome* b) {
        return a->fitness > b->fitness;
    });
    
    return genomes.front();
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
    
    clear();
    
    genomes = babies;
}
