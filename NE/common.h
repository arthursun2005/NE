//
//  common.h
//  NE
//
//  Created by Arthur Sun on 8/23/19.
//  Copyright © 2019 Arthur Sun. All rights reserved.
//

#ifndef common_h
#define common_h

#include <cmath>
#include <functional>
#include <cfloat>
#include <random>
#include <iostream>

typedef float float32;
typedef double float64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

static std::mt19937_64 ne_generator = std::mt19937_64(std::chrono::high_resolution_clock::now().time_since_epoch().count());

inline uint64 rand64() {
    return ne_generator();
}

inline float64 random(float64 a, float64 b) {
    return std::uniform_real_distribution<float64>(a, b)(ne_generator);
}

inline float64 gaussian_random(float64 a, float64 b) {
    return std::normal_distribution<float64>(a, b)(ne_generator);
}

inline uint64 random(uint64 a, uint64 b) {
    return random(0.0, 1.0) * (b - a) + a;
}

template <class T, class C>
inline void insert_in_order(std::vector<T>* a, const T& x, const C& compare) {
    typename std::vector<T>::iterator end = a->end();
    
    while(end-- != a->begin()) {
        if(compare(*end, x)) break;
    }
    
    ++end;
    a->insert(end, x);
}

#endif /* common_h */
