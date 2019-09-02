//
//  ne.h
//  NE
//
//  Created by Arthur Sun on 8/23/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef ne_h
#define ne_h

#include "common.h"
#include <fstream>

#define ne_reduce(x) (x / (1.0 + fabs(x)))

class ne_function
{
    
protected:
    
    float64* values;
    
    void _initialize_and_get(const ne_function* function, uint64 size) {
        uint64 bytes = sizeof(float64) * size;
        values = (float64*)::operator new(bytes);
        std::memcpy(values, function->values, bytes);
    }
    
public:
    
    ne_function(uint64 size = 0){
        values = (float64*)::operator new(sizeof(float64) * size);
    }
    
    ne_function(const ne_function* function, uint64 size) {
        _initialize_and_get(function, size);
    }
    
    ne_function(const ne_function& function) = delete;
    
    ne_function& operator = (const ne_function& function) = delete;
    
    ~ne_function() {
        ::operator delete(values);
    }
    
    static ne_function* crossover(const ne_function* a, const ne_function* b, uint64 size) {
        ne_function* f = new ne_function(size);
        
        for(uint64 i = 0; i < size; ++i) {
            f->values[i] = 0.5 * (a->values[i] + b->values[i]);
        }
        
        return f;
    }
    
    static float64 distance(const ne_function* a, const ne_function* b, uint64 size) {
        float64 d = 0.0;
        float64 q;
        
        for(uint64 i = 0; i < size; ++i) {
            q = b->values[i] - a->values[i];
            d += q * q;
        }
        
        return sqrt(d / (float64) size);
    }
    
    float64 evaluate(float64 x, uint64 size) const {
        x = ne_reduce(x);
        x = 0.5 * size * (x + 1.0);
        uint64 i = (uint64) x;
        uint64 j = i + 1;
        return values[i] * ((float64) j - x) + values[j] * (x - (float64) i);
    }
    
    void randomlize(uint64 size) {
        for(uint64 i = 0; i < size; ++i) {
            values[i] = gaussian_random();
        }
    }
    
    void mutate(uint64 size, float64 rate, float64 power) {
        for(uint64 i = 0; i < size; ++i) {
            if(random(0.0, 1.0) < rate)
                values[i] += random(-power, power);
        }
    }
    
};

#endif /* ne_h */
