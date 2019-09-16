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

struct ne_population
{
    ne_float average_rank;
    
    ne_settings settings;
    std::vector<ne_genome*> genomes;
    
    inline ne_population(const ne_settings& settings) : settings(settings) {}
    
    inline ne_population(const ne_population& population) {
        *this = population;
    }
    
    ne_population& operator = (const ne_population& population);
    
    inline ~ne_population() {
        clear();
    }
    
    inline void clear() {
        for(ne_genome* g : genomes)
            delete g;
    }
    
    void initialize();
    
    ne_genome* analyse();
    void reproduce();
    
    ne_genome* breed(ne_genome* g);
    
};

#endif /* population_h */
