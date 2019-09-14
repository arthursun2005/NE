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
#include <iostream>

typedef float float32;
typedef double float64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

template <class T, class C>
inline void insert_in_order(std::vector<T>* a, const T& x, C& compare) {
    typename std::vector<T>::iterator end = a->end();
    
    while(end-- != a->begin()) {
        if(compare(*end, x)) break;
    }
    
    ++end;
    a->insert(end, x);
}

#endif /* common_h */
