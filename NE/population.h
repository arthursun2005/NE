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

struct ne_population
{
    ne_float average_fitness;
    
    ne_settings settings;
    std::vector<ne_genome*> genomes;
    
    inline ne_population(const ne_settings& _settings) : settings(_settings) {
        genomes.resize(settings.population);
        
        for(ne_genome*& g : genomes) {
            g = new ne_genome(&settings);
            g->mutate_add_link();
        }
    }
    
    inline ne_population(const ne_population& population) : settings(population.settings) {
        genomes.resize(settings.population);
        
        for(ne_uint i = 0; i != settings.population; ++i) {
            genomes[i] = new ne_genome(*population.genomes[i]);
            genomes[i]->settings = &settings;
        }
    }
    
    inline ne_population(std::ifstream& is) {
        is.read((char*)&settings, sizeof(settings));
        genomes.resize(settings.population);
        for(ne_genome*& g : genomes)
            g = new ne_genome(is);
    }
    
    ne_population& operator = (const ne_population& population) = delete;
    
    inline ~ne_population() {
        for(ne_genome* g : genomes)
            delete g;
    }
    
    ne_genome* analyse();
    void reproduce();
    
    inline ne_genome* breed(ne_genome* g) {
        ne_genome* baby = new ne_genome(*g);
        baby->mutate();
        return baby;
    }
    
    inline void write(std::ofstream& os) const {
        os.write((char*)&settings, sizeof(settings));
        for(ne_genome* g : genomes)
            g->write(os);
    }
    
};

#endif /* population_h */
