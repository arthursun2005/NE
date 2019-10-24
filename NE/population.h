//
//  population.h
//  NE
//
//  Created by Arthur Sun on 8/24/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef population_h
#define population_h

#include "genome.h"
#include <iostream>

struct ne_settings {
    double mutate_add_prob;
    size_t population;
    
    ne_settings() {}
    
    ne_settings(double mutate_add_prob, double species_distance, size_t population) : mutate_add_prob(mutate_add_prob), population(population) {}
    
    ne_settings(std::ifstream& is) {
        is >> mutate_add_prob >> population;
    }
};

struct ne_species {
    std::vector<ne_genome*> genomes;
    size_t offsprings;
    double fitness;
};

inline double ne_distance(ne_genome* g1, ne_genome* g2) {
    std::vector<ne_link*>::iterator i1 = g1->links.begin();
    std::vector<ne_link*>::iterator i2 = g2->links.begin();
    std::vector<ne_link*>::iterator e1 = g1->links.end();
    std::vector<ne_link*>::iterator e2 = g2->links.end();
    
    double e = 0.0;
    
    while(i1 != e1 || i2 != e2) {
        if(i1 == e1 || (i2 != e2 && (*i2)->id < (*i1)->id)) {
            e += (*i2)->weight * (*i2)->weight;
            ++i2;
            continue;
        }
        
        if(i2 == e2 || (i1 != e1 && (*i1)->id < (*i2)->id)) {
            e += (*i1)->weight * (*i1)->weight;
            ++i1;
            continue;
        }
        
        if((*i1)->id == (*i2)->id) {
            double d = (*i2)->weight - (*i1)->weight;
            e += d * d;
            ++i1;
            ++i2;
            continue;
        }
    }
    
    return e;
}

struct ne_population {
    double fitness;
    size_t link_ids;
    
    ne_settings settings;
    std::vector<ne_genome*> genomes;
    std::vector<ne_species*> species;
    
    ne_population(const ne_settings& _settings, size_t input_size, size_t output_size) : settings(_settings), link_ids(0) {
        genomes.resize(settings.population);
        
        for(ne_genome*& g : genomes) {
            g = new ne_genome(input_size, output_size);
            g->mutate_add_link(&link_ids);
        }
    }
    
    ne_population(const ne_population& population) : settings(population.settings), link_ids(population.link_ids) {
        genomes.resize(settings.population);
        
        for(size_t i = 0; i != settings.population; ++i) {
            genomes[i] = new ne_genome(*population.genomes[i]);
        }
    }
    
    ne_population(std::ifstream& is) {
        is.read((char*)&settings, sizeof(settings));
        is.read((char*)&link_ids, sizeof(link_ids));
        genomes.resize(settings.population);
        for(ne_genome*& g : genomes)
            g = new ne_genome(is);
    }
    
    ne_population& operator = (const ne_population& population) = delete;
    
    ~ne_population() {
        for(ne_genome* g : genomes)
            delete g;
        
        for(ne_species* q : species)
            delete q;
    }
    
    ne_genome* analyse() {
        find_species();
        
        fitness = 0.0;
        
        for(ne_species* q : species) {
            q->fitness = 0.0;
            
            for(ne_genome* g : q->genomes) {
                g->fitness = fmax(0.0, g->fitness);
                q->fitness += g->fitness;
            }
            
            fitness += q->fitness / (double) q->genomes.size();
        }
        
        size_t sum = settings.population;
        
        if(fitness != 0.0) {
            for(ne_species* q : species) {
                q->offsprings = (size_t)floor(settings.population * q->fitness / ((double) q->genomes.size() * fitness));
                sum -= q->offsprings;
            }
            
            std::sort(species.begin(), species.end(), [] (ne_species* a, ne_species* b) {
                return a->fitness * b->genomes.size() > b->fitness * a->genomes.size();
            });
        }
        
        size_t i = 0;
        while(sum-- != 0) {
            ++species[i]->offsprings;
            i = (i + 1) % species.size();
        }
        
        return *std::min_element(genomes.begin(), genomes.end(), [] (ne_genome* a, ne_genome* b) {
            return a->fitness > b->fitness;
        });
    }
    
    void reproduce() {
        std::vector<ne_genome*> babies;
        
        for(ne_species* q : species) {
            size_t sum = q->offsprings;
            
            if(q->fitness != 0.0) {
                for(ne_genome* g : q->genomes) {
                    size_t offsprings = (size_t)floor(q->offsprings * g->fitness / q->fitness);
                    for(size_t n = 0; n != offsprings; ++n) {
                        babies.push_back(breed(g));
                    }
                    sum -= offsprings;
                }
                
                std::sort(q->genomes.begin(), q->genomes.end(), [] (ne_genome* a, ne_genome* b) {
                    return a->fitness > b->fitness;
                });
            }
            
            size_t i = 0;
            while(sum-- != 0) {
                babies.push_back(breed(q->genomes[i]));
                i = (i + 1) % q->genomes.size();
            }
        }
        
        for(ne_genome* g : genomes)
            delete g;
        
        genomes = babies;
    }
    
    ne_genome* breed(ne_genome* g) {
        ne_genome* baby = new ne_genome(*g);
        baby->mutate(settings.mutate_add_prob, &link_ids);
        return baby;
    }
    
    void write(std::ofstream& os) const {
        os.write((char*)&settings, sizeof(settings));
        os.write((char*)&link_ids, sizeof(link_ids));
        for(ne_genome* g : genomes)
            g->write(os);
    }
    
    void find_species() {
        for(ne_species* q : species)
            delete q;
        
        species.clear();
        
        for(ne_genome* g : genomes) {
            ne_species* sp = nullptr;
            
            for(ne_species* q : species) {
                if(ne_distance(q->genomes.front(), g) <= 0.0) {
                    sp = q;
                    break;
                }
            }
            
            if(!sp) {
                sp = new ne_species();
                species.push_back(sp);
            }
            
            sp->genomes.push_back(g);
        }
    }
    
};

#endif /* population_h */
