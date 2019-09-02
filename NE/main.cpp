//
//  main.cpp
//  NE
//
//  Created by Arthur Sun on 8/23/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include <iostream>
#include "population.h"

ne_population* population;

#define time_limit 1000

#define time_step 0.01f

int gens = 256;
int pop = 256;

int trials = 1;

struct Obj
{
    static const uint64 input_size;
    static const uint64 output_size;
    
    double reward;
    
    virtual void run(ne_genome* gen) = 0;
};

struct Pendulum : public Obj
{
    static const uint64 input_size = 5;
    static const uint64 output_size = 1;
    
    double x;
    double vx;
    double a;
    double va;
    
    double g;
    double m_c;
    double m_p;
    double m;
    double l;
    double f;
    double b;
    
    double xt = 2.4;
    
    Pendulum() {
        g = 9.8;
        m_c = 0.5;
        m_p = 0.5;
        m = m_c + m_p;
        l = 0.6;
        f = 20.0;
        b = 0.1;
    }
    
    void reset() {
        double stdev = 0.1;
        
        x = gaussian_random() * stdev;
        vx = gaussian_random() * stdev;
        a = gaussian_random() * stdev + M_PI;
        va = gaussian_random() * stdev;
        
        x = random(-0.75, 0.75) * xt;
    }
    
    void step(double dt, ne_genome* gen) {
        ne_input* inputs = gen->get_inputs();
        ne_output* outputs = gen->get_outputs();
        
        double c = cos(a);
        double s = sin(a);
        
        inputs[0].value = vx;
        inputs[1].value = x;
        inputs[2].value = c;
        inputs[3].value = s;
        inputs[4].value = va;
        
        gen->run();
        
        double action = outputs[0].value;
        
        action = action < -1.0 ? -1.0 : (action > 1.0 ? 1.0 : action);
        
        action *= f;
        
        double va2 = va * va;
        double sc = s * c;
        double c2 = c * c;
        
        double vvx = (-2.0 * m_p * l * va2 * s + 3.0 * m_p * g * sc + 4.0 * action - 4.0 * b * vx) / (4.0 * m - 3.0 * m_p * c2);
        double vva = (-3.0 * m_p * l * va2 * sc + 6.0 * m * g * s + 6.0 * (action - b * vx) * c) / (4.0 * l * m - 3.0 * m_p * l * c2);
        
        vx = vx + vvx * dt;
        va = va + vva * dt;
        
        x = x + vx * dt;
        a = a + va * dt;
        
        double q = x / xt;
        
        q = q > 1.0 ? 1.0 : q;
        q = q < -1.0 ? -1.0 : q;
        
        reward += 0.5 * (cos(a) + 1.0) * (cos(q * M_PI * 0.5));
    }
    
    void run(ne_genome* g) {
        reward = 0.0;
        
        for(int q = 0; q < trials; ++q) {
            reset();
            
            for(int i = 0; i < time_limit; ++i) {
                step(time_step, g);
            }
        }
        
        reward /= (double) trials;
    }
    
};

typedef Pendulum obj_type;

std::vector<obj_type> objs(pop);

void initialize() {
    population = new ne_population(obj_type::input_size, obj_type::output_size, pop);
    population->initialize();
}

int main(int argc, const char * argv[]) {
    initialize();
    
    ne_genome* best = nullptr;
    
    for(int n = 0; n < gens; ++n) {
        for(int i = 0; i < pop; ++i) {
            objs[i].run((*population)[i]);
            
            (*population)[i]->fitness = objs[i].reward;
        }
        
        std::cout << "Generation: " << n << std::endl;
        
        best = population->select();
        
        std::cout << std::endl << std::endl << std::endl;
        
        std::cout << "fitness: " << best->fitness << std::endl;
        
        std::cout << std::endl << std::endl << std::endl;
        
        for(ne_species* sp : population->species) {
            std::cout << "Species: " << sp->avg_fitness << "  offsprings: " << sp->offsprings << "  size: " << sp->genomes.size() << std::endl;
        }
        
        std::cout << n << "\t" << best->fitness << "\t" << std::endl;
        
        population->reproduce();
    }
    
    delete population;
    
    return 0;
}
