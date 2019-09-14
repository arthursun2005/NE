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
    ne_uint gs = genes.size();
    
    if(gs != 0) {
        ne_gene* gene = genes[ne_random() % gs];
        
        if(!gene->enabled() || gene->i->id == 0) return;
        
        ne_innovation innovation = params.add(gene, true);
        
        ne_node* node = new ne_node();
        node->id = innovation.node_id;
        insert(node);
        
        ne_gene* gene1 = new ne_gene(gene->i, node);
        
        gene1->id = innovation.gene_id;
        gene1->weight = 1.0;
        
        insert(gene1);
        
        ne_gene* gene2 = new ne_gene(node, gene->j);
        
        gene2->id = innovation.gene_id + 1;
        gene2->weight = gene->weight;
        
        insert(gene2);
        
        gene->disable();
    }
}

void ne_genome::mutate_add_gene(ne_params &params) {
    ne_uint size = nodes.size() - 1;
    
    ne_gene q(nodes[ne_random(0, size)], nodes[ne_random(input_size, size)]);
    
    ne_gene_set::iterator it = gene_set.find(&q);
    if(it != gene_set.end()) {
        if(!(*it)->enabled()) {
            (*it)->weight = ne_random(-2.0, 2.0);
        }
    }else{
        ne_gene* gene = new ne_gene(q);
        
        ne_innovation innovation = params.add(gene, false);
        gene->id = innovation.gene_id;
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

ne_float ne_genome::distance(const ne_genome *a, const ne_genome *b, const ne_params &params) {
    ne_float d = 0.0;
    ne_uint align = 0;
    ne_uint miss = 0;
    
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
    
    return miss * params.compat_gene + (align == 0 ? 0.0 : (d * params.compat_weight / (ne_float) align));
}

ne_population& ne_population::operator = (const ne_population& population) {
    clear();
    
    params = population.params;
    
    for(ne_species* sp : population.species) {
        ne_species* q = new ne_species();
        for(ne_genome* g : sp->genomes) {
            q->genomes.push_back(new ne_genome(*g));
        }
    }
    
    return *this;
}

void ne_population::initialize() {
    clear();
    
    for(ne_uint i = 0; i < params.population; ++i) {
        ne_genome* g = new ne_genome();
        g->reset(params);
        add(g);
    }
    
    params.node_ids = params.input_size + 1;
}

void ne_population::add(ne_genome *g) {
    ne_species* sp = nullptr;
    
    for(ne_species* s : species) {
        ne_genome* j = s->genomes.front();
        ne_float ts = ne_genome::distance(g, j, params);
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
    
    ne_uint i1 = ne_random(0, sp->parents - 1);
    
    if(ne_random(0.0, 1.0) < params.mutate_only_prob || sp->parents == 1) {
        baby = new ne_genome(*sp->genomes[i1]);
    }else{
        ne_uint i2 = ne_random(0, sp->parents - 1);
        baby = ne_genome::crossover(sp->genomes[i1], sp->genomes[i2], params);
    }
    
    ne_mutate(baby, params);

    return baby;
}

ne_genome* ne_population::select() {
    ne_uint offsprings = 0;
    ne_float total_fitness = 0.0;
    ne_genome* best = species.front()->genomes.front();
    
    for(ne_species* sp : species) {
        std::sort(sp->genomes.data(), sp->genomes.data() + sp->genomes.size(), ne_genome::sort);
        
        if(sp->genomes.front()->fitness > best->fitness)
            best = sp->genomes.front();
    }
    
    for(ne_species* sp : species) {
        ne_uint spsize = sp->genomes.size();
        
        sp->parents = (ne_uint)round(spsize * params.survive_thresh);
        if(sp->parents == 0) sp->parents = 1;
        
        sp->avg_fitness = 0.0;
        
        for(ne_uint i = 0; i < spsize; ++i) {
            if(isnan(sp->genomes[i]->fitness))
                sp->genomes[i]->fitness = 0.0;
            
            if(i < sp->parents)
                sp->avg_fitness += fmax(0.0, sp->genomes[i]->fitness);
        }
        
        sp->avg_fitness /= (ne_float) sp->parents;
        
        total_fitness += sp->avg_fitness;
    }
    
    if(total_fitness != 0.0) {
        for(ne_species* sp : species) {
            sp->offsprings = (ne_uint)floor(params.population * (sp->avg_fitness / total_fitness));
            offsprings += sp->offsprings;
        }
        
        std::sort(species.data(), species.data() + species.size(), ne_species::sort);
    }
    
    ne_uint leftover = params.population - offsprings;
    ne_uint ms = species.size() - 1;
    while(true) {
        if(leftover == 0) break;
        ++species[ne_random(0, ms)]->offsprings;
        --leftover;
    }
    
    return best;
}

void ne_population::reproduce() {
    params.set.clear();
    
    std::vector<ne_genome*> babies;
    
    for(ne_species* sp : species) {
        for(ne_uint n = 0; n != sp->offsprings; ++n) {
            babies.push_back(breed(sp));
        }
    }
    
    clear();
    
    for(ne_genome* g : babies) {
        add(g);
    }
}
