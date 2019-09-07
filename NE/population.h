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
    
    float64 avg_fitness;
    
    uint64 parents;
    uint64 offsprings;
    
    static inline bool sort(const ne_species* a, const ne_species* b) {
        return a->avg_fitness > b->avg_fitness;
    }
};

class ne_population
{
    
public:
    
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
        
    uint64 parents;
    
    std::vector<ne_species*> species;
    std::vector<ne_genome*> genomes;
    
protected:
    
    inline void clear() {
        for(ne_species* sp : species) {
            delete sp;
        }
                
        for(ne_genome* g : genomes) {
            delete g;
        }
        
        species.clear();
        genomes.clear();
    }
    
    ne_genome* breed(ne_species* sp);
    void add(ne_genome* g);
    
};

#endif /* population_h */
