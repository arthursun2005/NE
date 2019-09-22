//
//  main.cpp
//  NE
//
//  Created by Arthur Sun on 8/23/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include "population.h"

ne_population* population;

#define time_limit 1000

#define time_step 0.01f

int gens;
ne_settings settings;

struct Pendulum
{
    static const ne_uint input_size = 4;
    static const ne_uint output_size = 1;
    
    ne_float x;
    ne_float vx;
    ne_float a;
    ne_float va;
    
    ne_float g;
    ne_float m_c;
    ne_float m_p;
    ne_float m;
    ne_float l;
    ne_float f;
    ne_float b;
    
    ne_float xt = 10.0;
    
    ne_float fitness;
    
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
        x = ne_random(-2.0, 2.0);
        vx = ne_random(-1.0, 1.0);
        a = 0.07;//M_PI * 0.5 + ne_random(0.0, M_PI);
        va = ne_random(-M_PI, M_PI);
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
            
            inputs[0]->value = 1.0;
            inputs[1]->value = x / xt;
            inputs[2]->value = c;
            inputs[3]->value = s;
            
            gen->activate();
            
            action = outputs[0]->value * 2.0 - 1.0;
            
            action *= f;
            
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
            
            ne_float f1 = std::max(cos(a), 0.0);
            ne_float f2 = (xt - fabs(x)) / xt;
            
            fitness += f1 + (f1 * f2);
            
            if(p) {
                std::cout << x << ", " << a << ", " << std::endl;
            }
        }
        
        fitness /= 2000.0;
    }
    
};


struct XOR
{
    static const ne_uint input_size = 3;
    static const ne_uint output_size = 1;
    
    ne_float fitness;
    
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
                gen->activate();
                
                double d = outputs[0]->value - c;
                fitness += 1.0 - d * d;
                
                if(p) {
                    std::cout << outputs[0]->value << std::endl;
                }
            }
        }
        
        fitness *= 0.25;
    }
};

struct Game2048
{
    static const ne_uint input_size = 17;
    static const ne_uint output_size = 4;
    
    ne_float fitness;
    
    ne_uint grid[16];
    
    inline ne_uint& get(int x, int y) {
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
        }
        
        grid[idx[ne_random() % idx.size()]] = ne_random(0.0, 1.0) < 0.9 ? 2 : 4;
    }
    
    ne_uint move_left(bool& moved) {
        ne_uint score = 0;
        for(int y = 0; y < 4; ++y) {
            for(int x = 0; x < 4; ++x) {
                ne_uint v1 = get(x, y);
                if(v1 == 0) continue;
                
                for(int i = x + 1; i < 4; ++i) {
                    ne_uint v2 = get(i, y);
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
    
    ne_uint move_right(bool& moved) {
        ne_uint score = 0;
        for(int y = 0; y < 4; ++y) {
            for(int x = 3; x >= 0; --x) {
                ne_uint v1 = get(x, y);
                if(v1 == 0) continue;
                
                for(int i = x - 1; i >= 0; --i) {
                    ne_uint v2 = get(i, y);
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
    
    ne_uint move_up(bool& moved) {
        ne_uint score = 0;
        for(int x = 0; x < 4; ++x) {
            for(int y = 0; y < 4; ++y) {
                ne_uint v1 = get(x, y);
                if(v1 == 0) continue;
                
                for(int i = y + 1; i < 4; ++i) {
                    ne_uint v2 = get(x, i);
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
    
    ne_uint move_down(bool& moved) {
        ne_uint score = 0;
        for(int x = 0; x < 4; ++x) {
            for(int y = 3; y >= 0; --y) {
                ne_uint v1 = get(x, y);
                if(v1 == 0) continue;
                
                for(int i = y - 1; i >= 0; --i) {
                    ne_uint v2 = get(x, i);
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
                std::cout << std::setw(5) << get(x, y) << " ";
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
                    inputs[i + 1]->value = grid[i];
                }
                
                gen->flush();
                gen->activate();
                
                std::vector<int> choices(4);
                for(int i = 0; i < 4; ++i) choices[i] = i;
                
                std::sort(choices.data(), choices.data() + 4, [=] (int a, int b) {
                    return outputs[a]->value > outputs[b]->value;
                });
                
                bool moved = false;
                
                ne_uint i = 0;
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

struct DIR
{
    static const ne_uint input_size = 2;
    static const ne_uint output_size = 2;
    
    ne_float fitness;
    
    void run(ne_genome* gen, bool p) {
        fitness = 0.0;
        
        ne_node** inputs = gen->inputs();
        ne_node** outputs = gen->outputs();
        
        ne_float a = 0.0;
        ne_uint q = 200;
        ne_float d;
        for(ne_uint n = 0; n < q; ++n) {
            inputs[0]->value = 1.0;
            inputs[1]->value = a;
            
            gen->flush();
            gen->activate();
            //gen->adapt(0.0001);
            
            d = outputs[0]->value * 2.0 - 1.0 - cos(a);
            fitness += (1.0 - d * d) * 0.5;
            
            d = outputs[1]->value * 2.0 - 1.0 - sin(a);
            fitness += (1.0 - d * d) * 0.5;
            
            a += 0.05;
        }
        
        fitness /= (ne_float) q;
    }
};

struct GOL
{
    static const ne_uint input_size = 2;
    static const ne_uint output_size = 1;
    
    ne_float fitness;
    
    void run(ne_genome* gen, bool p) {
        fitness = 0.0;
        
        ne_node** inputs = gen->inputs();
        ne_node** outputs = gen->outputs();
        
        for(ne_uint a = 0; a <= 8; ++a) {
            inputs[0]->value = 1.0;
            inputs[1]->value = a;
            
            gen->flush();
            gen->activate();
            
            ne_float d = outputs[0]->value - (a == 1 ? 1.0 : 0.0);
            fitness += 1.0 - d * d;
            
            if(p) {
                std::cout << outputs[0]->value << std::endl;
            }
        }
        
        fitness /= (ne_float) 9;
    }
};

struct HANDDIGITS
{
    static const ne_uint input_size = 28 * 28 + 1;
    static const ne_uint output_size = 10;
    
    ne_float fitness;
    
    unsigned char* images;
    unsigned char* labels;
    
    int k, w, h;
    
    HANDDIGITS() {
        std::ifstream f1;
        f1.open("train-images-idx3-ubyte", std::fstream::ios_base::binary | std::fstream::ios_base::in);
        
        std::ifstream f2;
        f2.open("train-labels-idx1-ubyte", std::fstream::ios_base::binary | std::fstream::ios_base::in);
        
        assert(f1.is_open() && f2.is_open());
        
        f1.read((char*)&k, sizeof(k));
        f1.read((char*)&k, sizeof(k));
        f1.read((char*)&w, sizeof(w));
        f1.read((char*)&h, sizeof(h));
        
        k = __builtin_bswap32(k);
        w = __builtin_bswap32(w);
        h = __builtin_bswap32(h);
        
        images = (unsigned char*)::operator new(sizeof(unsigned char) * k * w * h);
        f1.read((char*)(images), sizeof(unsigned char) * k * w * h);
        
        int nl;
        
        f2.read((char*)&nl, sizeof(nl));
        f2.read((char*)&nl, sizeof(nl));
        
        nl = __builtin_bswap32(nl);
        assert(nl == k);
        
        labels = (unsigned char*)::operator new(sizeof(unsigned char) * nl);
        f2.read((char*)(labels), sizeof(unsigned char) * nl);
    }
    
    ~HANDDIGITS() {
        ::operator delete(images);
        ::operator delete(labels);
    }
    
    void load_image(int idx, ne_node** inputs) {
        int q = w * h;
        int a = idx * q;
        for(int i = 0; i < q; ++i) {
            inputs[i]->value = images[a + i] / 0x1p8;
        }
    }
    
    void run(ne_genome* gen, bool p) {
        fitness = 0.0;
        
        ne_node** inputs = gen->inputs();
        ne_node** outputs = gen->outputs();
        
        int trials = 10;
        int correct = 0;
        
        for(int n = 0; n < trials; ++n) {
            int i = (int)ne_random(0, (ne_uint)k - 1);
            int label = labels[i];
            inputs[0]->value = 1.0;
            load_image(i, inputs + 1);
            
            gen->flush();
            gen->activate();
            
            int h = 0;
            for(int j = 0; j < 10; ++j) {
                ne_float expected = label == j ? 1.0 : 0.0;
                ne_float d = outputs[j]->value - expected;
                fitness += (1.0 - d * d) * 0.1;
                
                if(outputs[j]->value > outputs[h]->value)
                    h = j;
                
                if(p) std::cout << outputs[j]->value << " ";
            }
            
            if(p) std::cout << "label: " << label << std::endl;
            
            if(h == label) ++correct;
        }

        fitness = correct;
    }
};

typedef DIR obj_type;

void initialize() {
    std::ifstream is("settings.ne");
    settings.read(is);
    is.close();
    settings.input_size = obj_type::input_size;
    settings.output_size = obj_type::output_size;
    population = new ne_population(settings);
}

int main(int argc, const char * argv[]) {
    if(argc == 1) {
        gens = 0x7fffffff;
        std::cout << "default number of generations: " << gens << std::endl;
    }else{
        gens = std::stoi(argv[1]);
    }
    
    initialize();
    
    ne_genome* best = nullptr;
    
    std::vector<ne_float> highs;
    
    obj_type obj;
    
    for(int n = 0; n < gens; ++n) {
        for(ne_genome* g : population->genomes) {
            obj.run(g, false);
            g->fitness = obj.fitness;
        }
        
        best = population->analyse();

        std::cout << n << " " << best->fitness << std::endl;
        
        if(n == gens - 1) {
            for(int q = 0; q < 1000; ++q) {
                obj.run(best, false);
                std::cout << "fitness: " << obj.fitness << std::endl;
            }
        }
        
        highs.push_back(best->fitness);
        
        population->reproduce();
    }
    
    std::cout << "Highs: " << std::endl;
    
    for(ne_uint i = 0; i < gens; ++i) {
        std::cout << i << "\t" << highs[i] << std::endl;
    }
    
    delete population;
    
    return 0;
}
