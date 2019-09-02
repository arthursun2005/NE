//
//  genome.h
//  NE
//
//  Created by Arthur Sun on 8/24/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef genome_h
#define genome_h

#include "ne.h"
#include <vector>

class ne_population;

struct ne_input
{
    float64 value;
    ne_function** functions;
};

struct ne_output
{
    float64 value;
    ne_function* function;
};

class ne_genome
{
    
public:
    
    ne_genome() {}
    
    ne_genome(const ne_genome& genome);
    ne_genome& operator = (const ne_genome& genome);
    
    ne_genome(ne_population* population);
    ~ne_genome();
    
    ne_input* get_inputs() {
        return inputs.data();
    }
    
    ne_output* get_outputs() {
        return outputs.data();
    }
    
    void mutate(float64 rate, float64 power);
    void run();
    
    float64 fitness;
    
    static inline bool compare(const ne_genome* a, const ne_genome* b) {
        return a->fitness > b->fitness;
    }
    
    static ne_genome* crossover(const ne_genome* a, const ne_genome* b);
    static float64 distance(const ne_genome* a, const ne_genome* b);
    
protected:
    
    void _initialize(const ne_genome& genome);
    
    std::vector<ne_input> inputs;
    std::vector<ne_output> outputs;
    
    uint64 accuracy;
    
};

#endif /* genome_h */
