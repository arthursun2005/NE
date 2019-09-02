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
    
    static inline bool compare(const ne_species* a, const ne_species* b) {
        return a->avg_fitness > b->avg_fitness;
    }
};

class ne_population
{
    
public:
    
    ne_population(uint64 input_size, uint64 output_size, uint64 population);
    
    ne_population(const char* file_name);
    
    ne_population(const ne_population&) = delete;
    ne_population& operator = (const ne_population&) = delete;
    
    ~ne_population();
    
    void initialize();
    
    ne_genome* select();
    void reproduce();
    
    inline ne_genome* operator [] (uint64 i) {
        return genomes[i];
    }
    
    friend class ne_genome;
    
    uint64 parents;
    
    std::vector<ne_species*> species;
    
protected:
    
    void clear();
    
    ne_genome* breed(ne_species* sp);
    void _add(ne_genome* g);
    
    uint64 input_size;
    uint64 output_size;
    
    uint64 population;
    uint64 accuracy;
    
    std::vector<ne_genome*> genomes;
    
};

#endif /* population_h */
