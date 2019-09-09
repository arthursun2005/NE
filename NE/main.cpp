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

int gens;
ne_params params;

struct Pendulum
{
    static const uint64 input_size = 3;
    static const uint64 output_size = 1;
    
    float64 x;
    float64 vx;
    float64 a;
    float64 va;
    
    float64 g;
    float64 m_c;
    float64 m_p;
    float64 m;
    float64 l;
    float64 f;
    float64 b;
    
    float64 xt = 10.0;
    
    float64 fitness;
    
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
        float64 range = 1.0;
        
        x = random(-range, range);
        vx = random(-range, range);
        a = random(-range, range) + M_PI;
        va = random(-range, range);
    }
    
    void run(ne_genome* gen, bool p) {
        fitness = 0.0;
        
        reset();
        gen->flush();
        
        ne_node** inputs = gen->inputs();
        ne_node** outputs = gen->outputs();
        
        for(int i = 0; i < time_limit; ++i) {
            double c = cos(a);
            double s = sin(a);
            
            double action = 0.0;
            
            if((i % 2) == 0) {
                //inputs[0]->value = vx;
                inputs[0]->value = 1.0;
                inputs[1]->value = x;
                inputs[2]->value = a;
                //inputs[2]->value = vx;
                //inputs[3]->value = va;
                //inputs[3]->value = s;
                //inputs[3]->value = va;
                
                gen->compute();
                
                action = outputs[0]->value * 2.0 - 1.0;
                
                action *= f;
            }
            
            double va2 = va * va;
            double c2 = c * c;

            double vva = (g * m * s + c * (action - m_p * l * va2 * s - b * vx))/(l * (m - m_p * c2));
            double vvx = (action + m_p * l * (vva * c - va2 * s) - b * vx) / m;
            
            vx = vx + vvx * time_step;
            va = va + vva * time_step;
            
            x = x + vx * time_step;
            a = a + va * time_step;
            
            if(x < -xt || x > xt)
                break;
            
            float64 f1 = std::max(cos(a), 0.0);
            float64 f2 = (xt - fabs(x)) / xt;
            
            fitness += f1 + (f1 * f2);
            
            if(p) {
                std::cout << x << ", " << a << ", " << std::endl;
            }
        }
    }
    
};


struct XOR
{
    static const uint64 input_size = 3;
    static const uint64 output_size = 1;
    
    float64 fitness;
    
    void run(ne_genome* gen, bool p) {
        fitness = 0.0;
        
        ne_node** inputs = gen->inputs();
        ne_node** outputs = gen->outputs();
        
        for(int a = 0; a < 2; ++a) {
            for(int b = 0; b < 2; ++b) {
                int c = a ^ b;
                
                inputs[0]->value = 1.0;
                inputs[1]->value = a;
                inputs[2]->value = b;
                
                gen->flush();
                gen->compute();
                
                double d = outputs[0]->value - c;
                fitness += 1.0 - d * d;
            }
        }
        
        fitness *= 0.25;
    }
};

struct Game2048
{
    static const uint64 input_size = 17;
    static const uint64 output_size = 4;
    
    float64 fitness;
    
    uint64 grid[16];
    
    inline uint64& get(int x, int y) {
        return grid[x + y * 4];
    }
    
    void reset() {
        for(int i = 0; i < 16; ++i) {
            grid[i] = 0;
        }
        
        add2();
    }
    
    void add2() {
        std::vector<int> idx;
        for(int i = 0; i < 16; ++i) {
            if(grid[i] == 0) idx.push_back(i);
            if(grid[i] == 2) idx.push_back(i);
        }
        
        grid[idx[rand64() % idx.size()]] += 2;
    }
    
    uint64 move_left(bool& moved) {
        uint64 score = 0;
        for(int y = 0; y < 4; ++y) {
            for(int x = 0; x < 4; ++x) {
                uint64 v1 = get(x, y);
                if(v1 == 0) continue;
                
                for(int i = x + 1; i < 4; ++i) {
                    uint64 v2 = get(i, y);
                    if(v1 == v2) {
                        score += v1 + v2;
                        moved = true;
                        
                        get(x, y) += get(i, y);
                        get(i, y) = 0;
                        
                        break;
                    }else if(v1 != v2 && v2 != 0) {
                        break;
                    }
                }
                
                int l = x;
                while(l != 0 && get(l - 1, y) == 0) {
                    get(l - 1, y) = get(l, y);
                    get(l, y) = 0;
                    --l;
                    
                    moved = true;
                }
            }
        }
        return score;
    }
    
    uint64 move_right(bool& moved) {
        uint64 score = 0;
        for(int y = 0; y < 4; ++y) {
            for(int x = 3; x >= 0; --x) {
                uint64 v1 = get(x, y);
                if(v1 == 0) continue;
                
                for(int i = x - 1; i >= 0; --i) {
                    uint64 v2 = get(i, y);
                    if(v1 == v2) {
                        score += v1 + v2;
                        moved = true;
                        
                        get(x, y) += get(i, y);
                        get(i, y) = 0;
                        
                        break;
                    }else if(v1 != v2 && v2 != 0) {
                        break;
                    }
                }
                
                int r = x;
                while(r != 3 && get(r + 1, y) == 0) {
                    get(r + 1, y) = get(r, y);
                    get(r, y) = 0;
                    ++r;
                    
                    moved = true;
                }
            }
        }
        return score;
    }
    
    uint64 move_up(bool& moved) {
        uint64 score = 0;
        for(int x = 0; x < 4; ++x) {
            for(int y = 0; y < 4; ++y) {
                uint64 v1 = get(x, y);
                if(v1 == 0) continue;
                
                for(int i = y + 1; i < 4; ++i) {
                    uint64 v2 = get(x, i);
                    if(v1 == v2) {
                        score += v1 + v2;
                        moved = true;
                        
                        get(x, y) += get(x, i);
                        get(x, i) = 0;

                        break;
                    }else if(v1 != v2 && v2 != 0) {
                        break;
                    }
                }
                
                int u = y;
                while(u != 0 && get(x, u - 1) == 0) {
                    get(x, u - 1) = get(x, u);
                    get(x, u) = 0;
                    --u;
                    
                    moved = true;
                }
            }
        }
        return score;
    }
    
    uint64 move_down(bool& moved) {
        uint64 score = 0;
        for(int x = 0; x < 4; ++x) {
            for(int y = 3; y >= 0; --y) {
                uint64 v1 = get(x, y);
                if(v1 == 0) continue;
                
                for(int i = y - 1; i >= 0; --i) {
                    uint64 v2 = get(x, i);
                    if(v1 == v2) {
                        score += v1 + v2;
                        moved = true;
                        
                        get(x, y) += get(x, i);
                        get(x, i) = 0;
                        
                        break;
                    }else if(v1 != v2 && v2 != 0) {
                        break;
                    }
                }
                
                int d = y;
                while(d != 3 && get(x, d + 1) == 0) {
                    get(x, d + 1) = get(x, d);
                    get(x, d) = 0;
                    ++d;
                    
                    moved = true;
                }
            }
        }
        return score;
    }
    
    int get_move() {
        for(int i = 0; i < 16; ++i) {
            if(grid[i] == 0) return -1;
        }
        
        for(int y = 0; y < 4; ++y) {
            for(int x = 0; x < 4; ++x) {
                if(x != 3 && get(x + 1, y) == get(x, y)) {
                    return 0;
                }
                if(y != 3 && get(x, y + 1) == get(x, y)) {
                    return 0;
                }
            }
        }
        
        return 1;
    }
    
    void print() {
        for(int y = 0; y < 4; ++y) {
            for(int x = 0; x < 4; ++x) {
                std::cout << get(x, y) << " ";
            }
            
            std::cout << std::endl;
        }
    }
    
    void run(ne_genome* gen, bool p) {
        fitness = 0.0;
        
        reset();
        
        ne_node** inputs = gen->inputs();
        ne_node** outputs = gen->outputs();
        
        while(true) {
            int m = get_move();
            
            if(p) {
                print();
                std::cout << std::endl;
            }
            
            if(m == 1) {
                break;
            }else{
                inputs[0]->value = 1.0;
                for(int i = 0; i < 16; ++i) {
                    inputs[i + 1]->value = log2(grid[i] + 1.0);
                }
                
                gen->flush();
                gen->compute();
                
                std::vector<int> choices(4);
                for(int i = 0; i < 4; ++i) choices[i] = i;
                
                std::sort(choices.data(), choices.data() + 4, [=] (int a, int b) {
                    return outputs[a]->value > outputs[b]->value;
                });
                
                bool moved = false;
                
                uint64 i = 0;
                while(!moved) {
                    int c = choices[i];
                    if(c == 0) {
                        fitness += move_up(moved);
                        if(p) std::cout << "up" << std::endl;
                    }else if(c == 1) {
                        fitness += move_down(moved);
                        if(p) std::cout << "down" << std::endl;
                    }else if(c == 2) {
                        fitness += move_left(moved);
                        if(p) std::cout << "left" << std::endl;
                    }else{
                        fitness += move_right(moved);
                        if(p) std::cout << "right" << std::endl;
                    }
                    ++i;
                }
                
                add2();
            }
        }
        
        
    }
};

typedef Pendulum obj_type;

std::vector<obj_type> objs;

void initialize() {
    params.input_size = obj_type::input_size;
    params.output_size = obj_type::output_size;
    population = new ne_population(params);
    population->initialize();
    objs.resize(params.population);
}

int main(int argc, const char * argv[]) {
    if(argc == 1) {
        gens = 64;
        std::cout << "default number of generations: " << gens << std::endl;
    }else{
        gens = std::stoi(argv[1]);
    }
    
    initialize();
    
    ne_genome* best = nullptr;
    
    std::vector<float64> highs;
    
    for(int n = 0; n < gens; ++n) {
        for(int i = 0; i < params.population; ++i) {
            objs[i].run(population->genomes[i], false);
            population->genomes[i]->fitness = objs[i].fitness;
        }
        
        std::cout << "Generation: " << n << std::endl;
        
        best = population->select();
        
        for(ne_species* sp : population->species) {
            std::cout << "Species: " << sp->avg_fitness << "  offsprings: " << sp->offsprings << "  size: " << sp->genomes.size() << std::endl;
        }
        
        std::cout << "fitness: " << best->fitness << std::endl;
        
        if(n == gens - 1) {
            objs[0].run(best, true);
            std::cout << "fitness: " << objs[0].fitness << std::endl;
            
            for(ne_node* node : best->nodes) {
                std::cout << "node: " << node->id << std::endl;
                for(ne_gene* gene : node->genes) {
                    std::cout << "gene: " << gene->id << " " << gene->weight << " " << gene->i->id << std::endl;
                }
                std::cout << std::endl;
            }
        }
        
        highs.push_back(best->fitness);
        
        population->reproduce();
    }
    
    std::cout << "Highs: " << std::endl;
    
    for(uint64 i = 0; i < gens; ++i) {
        std::cout << i << "\t" << highs[i] << std::endl;
    }
    
    delete population;
        
    return 0;
}
