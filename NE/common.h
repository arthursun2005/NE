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

inline uint32 rand32() {
    return arc4random();
}

inline uint64 rand64() {
    return rand32() | ((uint64)(rand32()) << 32);
}

inline float64 random(float64 a, float64 b) {
    return (rand64() / (float64) (0xffffffffffffffff)) * (b - a) + a;
}

inline uint64 random(uint64 a, uint64 b) {
    return random(0.0, 1.0) * (b - a) + a;
}

inline float64 gaussian_random() {
    static std::default_random_engine generator;
    static std::normal_distribution<float64> distribution(0.0, 1.0);
    return distribution(generator);
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
