//
//  population.cpp
//  NE
//
//  Created by Arthur Sun on 8/24/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "population.h"

ne_genome& ne_genome::operator = (const ne_genome &genome) {
    input_size = genome.input_size;
    output_size = genome.output_size;
    fitness = genome.fitness;
    
    clear();
    
    for(ne_gene* gene : genome.genes) {
        pass_down(new ne_gene(*gene));
    }
    
    initialize();
    
    return *this;
}

void ne_genome::compute() {
    for(ne_node* node : nodes) {
        if(node->id < input_size) continue;
        
        node->activated = false;
        node->sum = 0.0;
        
        for(ne_gene* gene : node->genes) {
            if(gene->i->activated && gene->enabled()) {
                node->activated = true;
                node->sum += gene->weight * gene->i->value;
            }
        }
    }
    
    for(ne_node* node : nodes) {
        if(node->activated) {
            node->value = ne_function(node->sum);
        }
    }
}

void ne_genome::mutate_add_node(ne_params &params) {
    uint64 gs = genes.size();
    
    if(gs != 0) {
        ne_gene* gene = genes[ne_random() % gs];
        
        if(!gene->enabled() || gene->i->id == 0) return;
        
        ne_node* node = new ne_node();
        node->id = params.node_ids++;
        insert(node);
        
        ne_gene* gene1 = new ne_gene(gene->i, node);
        
        gene1->id = params.gene_ids++;
        gene1->weight = 1.0;
        
        insert(gene1);
        
        ne_gene* gene2 = new ne_gene(node, gene->j);
        
        gene2->id = params.gene_ids++;
        gene2->weight = gene->weight;
        
        insert(gene2);
        
        gene->disable();
    }
}

void ne_genome::mutate_add_gene(ne_params &params) {
    uint64 size = nodes.size();
    
    ne_gene q(nodes[ne_random() % size], nodes[input_size + (ne_random() % (size - input_size))]);
    
    ne_gene_set::iterator it = gene_set.find(&q);
    if(it != gene_set.end()) {
        if(!(*it)->enabled()) {
            (*it)->weight = ne_random(-2.0, 2.0);
        }
    }else{
        ne_gene* gene = new ne_gene(q);
        
        gene->id = params.gene_ids++;
        gene->weight = ne_random(-2.0, 2.0);
        
        insert(gene);
    }
}

void ne_genome::mutate_weights(const ne_params &params) {
    for(ne_gene* gene : genes) {
        if(gene->enabled()) {
            gene->weight += ne_random(-params.mutate_weights_power, params.mutate_weights_power);
        }
    }
}

ne_genome* ne_genome::crossover(const ne_genome *a, const ne_genome *b, ne_params& params) {
    ne_genome* baby = new ne_genome();
    
    baby->input_size = a->input_size;
    baby->output_size = a->output_size;
    
    std::vector<ne_gene*>::const_iterator itA, itB;
    
    itA = a->genes.begin();
    itB = b->genes.begin();
    
    bool a_over_b = a->fitness > b->fitness;
    
    ne_gene gene;
    bool skip;
    
    while(itA != a->genes.end() || itB != b->genes.end()) {
        if(itA == a->genes.end()) {
            gene = **itB;
            skip = a_over_b;
            ++itB;
        }else if(itB == b->genes.end()) {
            gene = **itA;
            skip = !a_over_b;
            ++itA;
        }else if((*itA)->id == (*itB)->id) {
            gene = **itA;
            
            gene.weight = 0.5 * ((*itA)->weight + (*itB)->weight);
            
            skip = false;
            
            ++itA;
            ++itB;
        }else if((*itA)->id < (*itB)->id) {
            gene = **itA;
            skip = !a_over_b;
            ++itA;
        }else{
            gene = **itB;
            skip = a_over_b;
            ++itB;
        }
        
        if(skip) continue;
        
        ne_gene_set::iterator it = baby->gene_set.find(&gene);
        if(it == baby->gene_set.end()) {
            baby->pass_down(new ne_gene(gene));
        }
    }
    
    baby->initialize();
    
    return baby;
}

float64 ne_genome::distance(const ne_genome *a, const ne_genome *b, const ne_params &params) {
    float64 d = 0.0;
    uint64 align = 0;
    uint64 miss = 0;
    
    std::vector<ne_gene*>::const_iterator itA, itB;
    std::vector<ne_gene*>::const_iterator endA, endB;
    
    itA = a->genes.begin();
    itB = b->genes.begin();
    
    endA = a->genes.end();
    endB = b->genes.end();
    
    while(itA != endA || itB != endB) {
        if(itA == endA) {
            ++miss;
            ++itB;
        }else if(itB == endB) {
            ++miss;
            ++itA;
        }else if((*itA)->id == (*itB)->id) {
            d += fabs((*itB)->weight - (*itA)->weight);
            ++align;
            ++itA;
            ++itB;
        }else if((*itA)->id < (*itB)->id) {
            ++miss;
            ++itA;
        }else{
            ++miss;
            ++itB;
        }
    }
    
    return miss * params.compat_gene + (align == 0 ? 0.0 : (d * params.compat_weight / (float64) align));
}

ne_population& ne_population::operator = (const ne_population& population) {
    clear();
    
    params = population.params;
    
    genomes.resize(params.population);
    
    for(uint64 i = 0; i < params.population; ++i) {
        genomes[i] = new ne_genome(*population.genomes[i]);
        add(genomes[i]);
    }
    
    return *this;
}

void ne_population::initialize() {
    clear();
    
    genomes.resize(params.population);
    
    for(uint64 i = 0; i < params.population; ++i) {
        genomes[i] = new ne_genome();
        genomes[i]->reset(params);
        add(genomes[i]);
    }
    
    params.node_ids = params.input_size + 1;
}

void ne_population::add(ne_genome *g) {
    ne_species* sp = nullptr;
    
    for(ne_species* s : species) {
        ne_genome* j = s->genomes.front();
        float64 ts = ne_genome::distance(g, j, params);
        if(ts < params.compat_thresh) {
            sp = s;
            break;
        }
    }
    
    if(sp == nullptr) {
        sp = new ne_species();
        species.push_back(sp);
    }
    
    sp->genomes.push_back(g);
}

ne_genome* ne_population::breed(ne_species *sp) {
    ne_genome* baby;
    
    uint64 i1 = ne_random(0, sp->parents - 1);
    
    if(ne_random(0.0, 1.0) < params.mutate_only_prob || sp->parents == 1) {
        baby = new ne_genome(*sp->genomes[i1]);
    }else{
        uint64 i2 = ne_random(0, sp->parents - 1);
        baby = ne_genome::crossover(sp->genomes[i1], sp->genomes[i2], params);
    }
    
    ne_mutate(baby, params);

    return baby;
}

ne_genome* ne_population::select() {
    uint64 offsprings = 0;
    float64 total_fitness = 0.0;
    ne_genome* best = genomes[0];
    
    for(ne_species* sp : species) {
        std::sort(sp->genomes.data(), sp->genomes.data() + sp->genomes.size(), ne_genome::sort);
        
        if(sp->genomes[0]->fitness > best->fitness)
            best = sp->genomes[0];
    }
    
    for(ne_species* sp : species) {
        uint64 spsize = sp->genomes.size();
        
        sp->parents = (uint64)round(spsize * params.survive_thresh);
        if(sp->parents == 0) sp->parents = 1;
        
        sp->avg_fitness = 0.0;
        
        for(uint64 i = 0; i < spsize; ++i) {
            if(isnan(sp->genomes[i]->fitness))
                sp->genomes[i]->fitness = 0.0;
            
            if(i < sp->parents)
                sp->avg_fitness += fmax(0.0, sp->genomes[i]->fitness);
        }
        
        sp->avg_fitness /= (float64) sp->parents;
        
        total_fitness += sp->avg_fitness;
    }
    
    if(total_fitness != 0.0) {
        for(ne_species* sp : species) {
            sp->offsprings = (uint64)floor(params.population * (sp->avg_fitness / total_fitness));
            offsprings += sp->offsprings;
        }
        
        std::sort(species.data(), species.data() + species.size(), ne_species::sort);
    }
    
    uint64 leftover = params.population - offsprings;
    uint64 ms = species.size() - 1;
    while(true) {
        if(leftover == 0) break;
        ++species[ne_random(0, ms)]->offsprings;
        --leftover;
    }
    
    return best;
}

void ne_population::reproduce() {
    std::vector<ne_genome*> babies;
    
    for(ne_species* sp : species) {
        for(uint64 n = 0; n != sp->offsprings; ++n) {
            babies.push_back(breed(sp));
        }
    }
    
    clear();
    
    genomes = babies;
    
    for(ne_genome* g : genomes) {
        add(g);
    }
}
