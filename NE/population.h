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
    
    ne_settings(double mutate_add_prob, size_t population) : mutate_add_prob(mutate_add_prob), population(population) {}
    
    void read(std::ifstream& is) {
        is >> mutate_add_prob >> population;
    }
};

struct ne_population {
    double average_fitness;
    
    ne_settings settings;
    std::vector<ne_genome*> genomes;
    
    ne_population(const ne_settings& _settings, size_t input_size, size_t output_size) : settings(_settings) {
        genomes.resize(settings.population);
        
        for(ne_genome*& g : genomes) {
            g = new ne_genome(input_size, output_size);
        }
    }
    
    ne_population(const ne_population& population) : settings(population.settings) {
        genomes.resize(settings.population);
        
        for(size_t i = 0; i != settings.population; ++i) {
            genomes[i] = new ne_genome(*population.genomes[i]);
        }
    }
    
    ne_population(std::ifstream& is) {
        is.read((char*)&settings, sizeof(settings));
        genomes.resize(settings.population);
        for(ne_genome*& g : genomes)
            g = new ne_genome(is);
    }
    
    ne_population& operator = (const ne_population& population) = delete;
    
    ~ne_population() {
        for(ne_genome* g : genomes)
            delete g;
    }
    
    ne_genome* analyse() {
        average_fitness = 0.0;
        
        for(ne_genome* g : genomes) {
            g->fitness = fmax(0.0, g->fitness);
            average_fitness += g->fitness;
        }
        
        average_fitness /= (double) settings.population;
        
        std::sort(genomes.begin(), genomes.end(), [] (ne_genome* a, ne_genome* b) {
            return a->fitness > b->fitness;
        });
        
        return genomes.front();
    }
    
    void reproduce() {
        std::vector<ne_genome*> babies;
        
        if(average_fitness != 0.0) {
            for(ne_genome* g : genomes) {
                size_t offsprings = (size_t)floor(g->fitness / average_fitness);
                
                for(size_t n = 0; n != offsprings; ++n)
                    babies.push_back(breed(g));
            }
        }
        
        size_t leftover = settings.population - babies.size();
        for(ne_genome* g : genomes) {
            if(leftover <= 0) break;
            babies.push_back(breed(g));
            --leftover;
        }
        
        for(ne_genome* g : genomes)
            delete g;
        
        genomes = babies;
    }
    
    ne_genome* breed(ne_genome* g) {
        ne_genome* baby = new ne_genome(*g);
        baby->mutate(settings.mutate_add_prob);
        return baby;
    }
    
    void write(std::ofstream& os) const {
        os.write((char*)&settings, sizeof(settings));
        for(ne_genome* g : genomes)
            g->write(os);
    }
    
};

#endif /* population_h */
