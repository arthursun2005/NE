//
//  population.cpp
//  NE
//
//  Created by Arthur Sun on 8/24/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include "population.h"

ne_genome::ne_genome(ne_population* population) {
    inputs.resize(population->input_size);
    outputs.resize(population->output_size);
    
    accuracy = population->accuracy;
    
    for(ne_input& input : inputs) {
        input.functions = (ne_function**)::operator new(sizeof(ne_function*) * population->output_size);
        for(uint64 i = 0; i < population->output_size; ++i) {
            input.functions[i] = new ne_function(accuracy);
            input.functions[i]->randomlize(accuracy);
        }
    }
    
    for(ne_output& output : outputs) {
        output.function = new ne_function(accuracy);
        output.function->randomlize(accuracy);
    }
}

void ne_genome::_initialize(const ne_genome &genome) {
    uint64 input_size = genome.inputs.size();
    uint64 output_size = genome.outputs.size();
    
    inputs.resize(input_size);
    outputs.resize(output_size);
    
    accuracy = genome.accuracy;
    
    for(uint64 i = 0; i < input_size; ++i) {
        inputs[i].functions = (ne_function**)::operator new(sizeof(ne_function*) * output_size);
        for(uint64 j = 0; j < output_size; ++j) {
            inputs[i].functions[j] = new ne_function(genome.inputs[i].functions[j], accuracy);
        }
    }
    
    for(uint64 i = 0; i < output_size; ++i) {
        outputs[i].function = new ne_function(genome.outputs[i].function, accuracy);
    }
}

ne_genome::ne_genome(const ne_genome& genome) {
    _initialize(genome);
}

ne_genome& ne_genome::operator = (const ne_genome &genome) {
    this->~ne_genome();
    _initialize(genome);
    return *this;
}

ne_genome::~ne_genome() {
    uint64 output_size = outputs.size();
    
    for(ne_input& input : inputs) {
        for(uint64 i = 0; i < output_size; ++i) {
            delete input.functions[i];
        }
        
        ::operator delete(input.functions);
    }
    
    for(uint64 i = 0; i < output_size; ++i) {
        delete outputs[i].function;
    }
}

ne_genome* ne_genome::crossover(const ne_genome *a, const ne_genome *b) {
    ne_genome* genome = new ne_genome();
    
    uint64 input_size = a->inputs.size();
    uint64 output_size = b->outputs.size();
    
    genome->inputs.resize(input_size);
    genome->outputs.resize(output_size);
    
    genome->accuracy = a->accuracy;
    
    for(uint64 i = 0; i < input_size; ++i) {
        genome->inputs[i].functions = (ne_function**)::operator new(sizeof(ne_function*) * output_size);
        for(uint64 j = 0; j < output_size; ++j) {
            genome->inputs[i].functions[j] = ne_function::crossover(a->inputs[i].functions[j], b->inputs[i].functions[j], genome->accuracy);
        }
    }
    
    for(uint64 i = 0; i < output_size; ++i) {
        genome->outputs[i].function = ne_function::crossover(a->outputs[i].function, b->outputs[i].function, genome->accuracy);
    }
    
    return genome;
}

float64 ne_genome::distance(const ne_genome *a, const ne_genome *b) {
    float64 d = 0.0;
    
    uint64 input_size = a->inputs.size();
    uint64 output_size = b->outputs.size();

    for(uint64 i = 0; i < input_size; ++i) {
        for(uint64 j = 0; j < output_size; ++j) {
            d += ne_function::distance(a->inputs[i].functions[j], b->inputs[i].functions[j], a->accuracy);
        }
    }
    
    for(uint64 i = 0; i < output_size; ++i) {
        d += ne_function::distance(a->outputs[i].function, b->outputs[i].function, a->accuracy);
    }
    
    return d / ((float64) ((input_size + 1) * output_size));
}

void ne_genome::mutate(float64 rate, float64 power) {
    uint64 output_size = outputs.size();
    
    for(ne_input& input : inputs) {
        for(uint64 i = 0; i < output_size; ++i) {
            input.functions[i]->mutate(accuracy, rate, power);
        }
    }
    
    for(uint64 i = 0; i < output_size; ++i) {
        outputs[i].function->mutate(accuracy, rate, power);
    }
}

void ne_genome::run() {
    uint64 output_size = outputs.size();
    
    for(uint64 i = 0; i < output_size; ++i) {
        outputs[i].value = 0.0;
    }
    
    for(ne_input& input : inputs) {
        for(uint64 i = 0; i < output_size; ++i) {
            outputs[i].value += input.functions[i]->evaluate(input.value, accuracy);
        }
    }
    
    for(uint64 i = 0; i < output_size; ++i) {
        outputs[i].value = outputs[i].function->evaluate(outputs[i].value, accuracy);
    }
}

ne_population::ne_population(uint64 input_size, uint64 output_size, uint64 population) : input_size(input_size), output_size(output_size), population(population) {
    accuracy = 16;
}

void ne_population::clear() {
    for(ne_species* sp : species) {
        delete sp;
    }
    
    species.clear();
    
    for(ne_genome* g : genomes) {
        delete g;
    }
    
    genomes.clear();
}

void ne_population::initialize() {
    clear();
    
    genomes.resize(population);
    for(uint64 i = 0; i < population; ++i) {
        genomes[i] = new ne_genome(this);
    }
}

ne_population::~ne_population() {
    clear();
}

void ne_population::_add(ne_genome *g) {
    ne_species* sp = nullptr;
    float64 mc = 2000.0;
    
    for(ne_species* s : species) {
        ne_genome* j = s->genomes.front();
        float64 ts = ne_genome::distance(g, j);
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

ne_genome* ne_population::select() {
    for(ne_species* sp : species) {
        delete sp;
    }
    
    species.clear();
    
    ne_genome* best = genomes[0];
    
    for(ne_genome* g : genomes) {
        _add(g);
        
        if(g->fitness > best->fitness)
            best = g;
    }
    
    float64 total_fitness = 0.0;
    uint64 offsprings = 0;
    
    parents = 0;
    
    for(ne_species* sp : species) {
        uint64 spsize = sp->genomes.size();
        
        std::sort(sp->genomes.data(), sp->genomes.data() + spsize, ne_genome::compare);
        
        sp->parents = (uint64)ceil(spsize * 0.5);
        
        parents += sp->parents;
        
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
    
    if(total_fitness == 0.0)
        total_fitness = 1.0;
    
    for(ne_species* sp : species) {
        sp->offsprings = (uint64)(population * (sp->avg_fitness / total_fitness));
        offsprings += sp->offsprings;
    }
    
    std::sort(species.data(), species.data() + species.size(), ne_species::compare);
    
    uint64 leftover = population - offsprings;
    while(leftover != 0) {
        for(ne_species* sp : species) {
            if(leftover == 0) break;
            ++sp->offsprings;
            --leftover;
        }
    }
    
    return best;
}

ne_genome* ne_population::breed(ne_species *sp) {
    ne_genome* baby;
    
    uint64 i1 = random(0, sp->parents);
    
    if(random(0.0, 1.0) < 0.75) {
        uint64 i2 = random(0, sp->parents);
        baby = ne_genome::crossover(sp->genomes[i1], sp->genomes[i2]);
    }else{
        baby = new ne_genome(*sp->genomes[i1]);
    }
    
    if(random(0.0, 1.0) < 0.75)
        baby->mutate(0.75, 2.0);
    
    return baby;
}

void ne_population::reproduce() {
    for(ne_species* sp : species) {
        uint64 inv_size = 1.0 / (float64) sp->genomes.size();
        
        for(ne_genome* g : sp->genomes) {
            g->fitness *= inv_size;
        }
    }
    
    std::sort(genomes.data(), genomes.data() + population, ne_genome::compare);
    
    std::vector<ne_genome*> babies;
    
    for(ne_species* sp : species) {
        if(sp->offsprings != 0) {
            for(uint64 n = 1; n != sp->offsprings; ++n) {
                babies.push_back(breed(sp));
            }
            
            babies.push_back(new ne_genome(*sp->genomes.front()));
        }
    }
        
    clear();
    
    genomes = babies;
}
