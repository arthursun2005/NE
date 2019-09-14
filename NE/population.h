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

struct ne_species
{
    std::vector<ne_genome*> genomes;
    
    ne_float avg_fitness;
    
    ne_uint parents;
    ne_uint offsprings;
    
    static inline bool sort(const ne_species* a, const ne_species* b) {
        return a->avg_fitness > b->avg_fitness;
    }
};

struct ne_population
{
    ne_population(const ne_params& params) : params(params) {}
    
    ne_population(const ne_population& population) {
        *this = population;
    }
    
    ne_population& operator = (const ne_population& population);
    
    ~ne_population() {
        clear();
    }
    
    void initialize();
    
    ne_genome* select();
    void reproduce();
    
    ne_params params;
    
    std::vector<ne_species*> species;
    
    inline void clear() {
        for(ne_species* sp : species) {
            delete sp;
        }
        
        species.clear();
    }
    
    ne_genome* breed(ne_species* sp);
    void add(ne_genome* g);
};

#endif /* population_h */
