//
//  population.cpp
//  NE
//
//  Created by Arthur Sun on 8/24/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "population.h"

void ne_mutate(ne_genome* g, ne_params& params) {
    if(random(0.0, 1.0) < params.mutate_add_node_prob) {
        g->mutate_add_node(params);
    }
    
    if(random(0.0, 1.0) < params.mutate_add_gene_prob) {
        g->mutate_add_gene(params);
    }
    
    if(random(0.0, 1.0) < params.mutate_function_prob) {
        g->mutate_functions(params);
    }
    
    if(random(0.0, 1.0) < params.mutate_weight_prob) {
        g->mutate_weights(params);
    }
    
    if(random(0.0, 1.0) < params.mutate_activation_prob) {
        g->activations++;
    }
}

void ne_genome::reset(const ne_params &params) {
    input_size = params.input_size;
    output_size = params.output_size;
    
    clear();
    
    for(uint64 i = 0; i < input_size; ++i) {
        ne_node* node = new ne_node(i);
        insert(node);
    }
    
    for(uint64 i = 0; i < output_size; ++i) {
        ne_node* node = new ne_node(i + input_size);
        insert(node);
    }
    
    for(uint64 i = 0; i < input_size; ++i) {
        for(uint64 j = 0; j < output_size; ++j) {
            uint64 q = input_size + j;
            ne_gene* gene = new ne_gene();
            
            gene->i = nodes[i];
            gene->j = nodes[q];
            gene->weight = gaussian_random();
            gene->id = genes.size();
            
            insert(gene);
        }
    }
    
    activations = 1;
}

ne_genome::ne_genome(const ne_genome& genome) {
    *this = genome;
}

ne_genome& ne_genome::operator = (const ne_genome &genome) {
    input_size = genome.input_size;
    output_size = genome.output_size;
    fitness = genome.fitness;
    activations = genome.activations;
    eliminated = genome.eliminated;
    
    clear();
    
    for(ne_gene* gene : genome.genes) {
        pass_down(new ne_gene(*gene));
    }
    
    return *this;
}

void ne_genome::mutate_add_node(ne_params &params) {
    uint64 gs = genes.size();
    
    if(gs != 0) {
        ne_gene* gene = genes[rand64() % gs];
        
        if(!gene->enabled()) return;
        
        ne_node* node = new ne_node(params.node_ids++);
        insert(node);
        
        ne_gene* gene1 = new ne_gene();
        
        gene1->i = gene->i;
        gene1->j = node;
        gene1->id = params.gene_ids++;
        gene1->weight = 1.0;
        
        insert(gene1);
        
        ne_gene* gene2 = new ne_gene();
        
        gene2->i = node;
        gene2->j = gene->j;
        gene2->id = params.gene_ids++;
        gene2->weight = gene->weight;
        
        insert(gene2);
        
        gene->disable();
    }
}

void ne_genome::mutate_add_gene(ne_params &params) {
    uint64 size = nodes.size();
    
    ne_gene q;
    q.i = nodes[rand64() % size];
    q.j = nodes[rand64() % size];
    
    ne_gene_set::iterator it = gene_set.find(&q);
    if(it != gene_set.end()) {
        if(!(*it)->enabled()) {
            (*it)->weight = gaussian_random();
        }
    }else{
        ne_gene* gene = new ne_gene(q);
        
        gene->id = params.gene_ids++;
        gene->weight = gaussian_random();
        
        insert(gene);
    }
}

void ne_genome::mutate_functions(ne_params &params) {
    for(ne_node* node : nodes) {
        if(random(0.0, 1.0) < params.mutate_function_rate)
            node->function.mutate(params);
    }
}

void ne_genome::mutate_weights(ne_params &params) {
    for(ne_gene* gene : genes) {
        if(gene->enabled() && random(0.0, 1.0) < params.mutate_weight_rate)
            gene->weight += random(-params.mutate_weight_power, params.mutate_weight_power);
    }
}

void ne_genome::crossover(const ne_genome *a, const ne_genome *b, ne_genome* baby, ne_params& params) {
    baby->clear();
    
    baby->input_size = a->input_size;
    baby->output_size = a->output_size;
    
    baby->activations = std::min(a->activations, b->activations);
    
    std::vector<ne_gene*>::const_iterator itA, itB;
    
    itA = a->genes.begin();
    itB = b->genes.begin();
    
    bool a_over_b = a->fitness > b->fitness;
    bool mate_avg = random(0.0, 1.0) < params.mate_avg_prob;
    
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
            
            if(mate_avg) gene.weight = 0.5 * ((*itA)->weight + (*itB)->weight);
            else gene.weight = (rand32() & 1) ? (*itA)->weight : (*itB)->weight;
            
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
    
    for(ne_node* node : baby->nodes) {
        ne_node_set::iterator nA, nB;
        nA = a->node_set.find(node);
        nB = b->node_set.find(node);
        if(nA != a->node_set.end() && nB != b->node_set.end()) {
            ne_function::crossover(&(*nA)->function, &(*nB)->function, &node->function);
        }
    }
}

float64 ne_genome::distance_nodes(const ne_genome *a, const ne_genome *b, const ne_params &params) {
    float64 d = 0.0;
    uint64 align = 0;
    uint64 miss = 0;
    
    std::vector<ne_node*>::const_iterator itA, itB;
    std::vector<ne_node*>::const_iterator endA, endB;
    
    itA = a->nodes.begin();
    itB = b->nodes.begin();
    
    endA = a->nodes.end();
    endB = b->nodes.end();
    
    while(itA != endA || itB != endB) {
        if(itA == endA) {
            ++miss;
            ++itB;
        }else if(itB == endB) {
            ++miss;
            ++itA;
        }else if((*itA)->id == (*itB)->id) {
            float64 q = ne_function::distance(&(*itA)->function, &(*itB)->function);
            d += q * q;
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
    
    return miss * params.compat_node + sqrt(d / (float64) align) * params.compat_function;
}

float64 ne_genome::distance_genes(const ne_genome *a, const ne_genome *b, const ne_params &params) {
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
            float64 q = (*itA)->weight - (*itB)->weight;
            d += q * q;
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
    
    return miss * params.compat_gene + sqrt(d / (float64) align) * params.compat_weight;
}

void ne_genome::flush() {
    uint64 size = nodes.size();
    
    for(uint64 i = 0; i < input_size; ++i) {
        nodes[i]->activated = true;
    }
    
    for(uint64 i = input_size; i < size; ++i) {
        nodes[i]->activated = false;
    }
}

void ne_genome::step() {
    uint64 n = 0;
    
    while(n != activations) {
        for(ne_node* node : nodes) {
            node->computed = false;
            node->sum = 0.0;
        }
        
        for(ne_gene* gene : genes) {
            if(gene->i->activated && gene->enabled()) {
                gene->j->computed = true;
                gene->j->sum += gene->i->value * gene->weight;
            }
        }
        
        for(ne_node* node : nodes) {
            node->activated = node->computed;
            
            if(node->activated) {
                node->value = node->function(node->sum);
            }
        }
        
        ++n;
    }
}

ne_population::ne_population(const ne_params& params) : params(params) {
}

ne_population::ne_population(const ne_population& population) {
    *this = population;
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
    
    params.node_ids = params.input_size + params.output_size;
    params.gene_ids = params.input_size * params.output_size;
}

void ne_population::add(ne_genome *g) {
    ne_species* sp = nullptr;
    float64 mc = params.compat_thresh;
    
    for(ne_species* s : species) {
        ne_genome* j = s->genomes.front();
        float64 ts = ne_genome::distance(g, j, params);
        if(ts < mc) {
            mc = ts;
            sp = s;
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
    
    uint64 i1 = random(0, sp->parents);
    
    if(random(0.0, 1.0) < params.mutate_only_prob || sp->parents == 1) {
        baby = new ne_genome(*sp->genomes[i1]);
        
        ne_mutate(baby, params);
    }else{
        baby = new ne_genome();
        
        uint64 i2 = random(0, sp->parents);
        ne_genome::crossover(sp->genomes[i1], sp->genomes[i2], baby, params);
        
        if(random(0.0, 1.0) >= params.mate_only_prob) {
            ne_mutate(baby, params);
        }
    }
    
    baby->eliminated = false;
    
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
        
        sp->parents = (uint64)ceil(spsize * params.survive_thresh);
        
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
            sp->offsprings = (uint64)(params.population * (sp->avg_fitness / total_fitness));
            offsprings += sp->offsprings;
        }
        
        std::sort(species.data(), species.data() + species.size(), ne_species::sort);
    }
    
    uint64 leftover = params.population - offsprings;
    while(leftover != 0) {
        for(ne_species* sp : species) {
            if(leftover == 0) break;
            ++sp->offsprings;
            --leftover;
        }
    }
    
    return best;
}

void ne_population::reproduce() {
    std::vector<ne_genome*> babies;
    
    for(ne_species* sp : species) {
        for(ne_genome* g : sp->genomes) {
            g->eliminated = true;
        }
        
        if(sp->offsprings != 0) {
            for(uint64 n = 1; n != sp->offsprings; ++n) {
                babies.push_back(breed(sp));
            }
            
            sp->genomes.front()->eliminated = false;
        }
    }
    
    for(ne_genome* g : babies) {
        add(g);
        genomes.push_back(g);
    }
    
    std::vector<ne_genome*>::iterator begin, end;
    std::vector<ne_species*>::iterator p = species.begin(), q = species.end();
    
    while(q-- != p) {
        begin = (*q)->genomes.begin();
        end = (*q)->genomes.end();
        
        while(end-- != begin) {
            if((*end)->eliminated)
                (*q)->genomes.erase(end);
        }
        
        if((*q)->genomes.empty()) {
            delete *q;
            species.erase(q);
        }
    }
    
    begin = genomes.begin();
    end = genomes.end();
    
    while(end-- != begin) {
        if((*end)->eliminated) {
            delete *end;
            genomes.erase(end);
        }
    }
}
