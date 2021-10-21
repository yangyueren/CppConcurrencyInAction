//
// Created by yryang on 2021/10/21.
//

#ifndef CPPTEST_QUICKSORT_SEQUENTIAL_H
#define CPPTEST_QUICKSORT_SEQUENTIAL_H

#include <algorithm>
#include <iostream>
#include <list>

namespace quicksort{
    template <typename T>
    std::list<T> quicksort_sequential(std::list<T> input){
        if (input.empty()){
            return input;
        }

        // Transfers the element pointed to by it from other
        // into *this. The element is inserted before the element pointed to by pos.
        std::list<T> result;
        result.splice(result.begin(), input, input.begin()); // only one element.

        T const &pivot = *result.begin();
        auto divid_point = std::partition(input.begin(), input.end(), [&](T const &t){return t < pivot;});

        std::list<T> lower_part;
        lower_part.splice(lower_part.end(), input, input.begin(), divid_point);

        auto new_lower(quicksort_sequential<T>(lower_part));
        auto new_higher(quicksort_sequential<T>(input));

        result.splice(result.end(), new_higher);
        result.splice(result.begin(), new_lower);
        return result;

    }
}



#endif //CPPTEST_QUICKSORT_SEQUENTIAL_H
