//
// Created by yryang on 2021/10/21.
//

#ifndef CPPTEST_QUICKSORT_USING_FUTURES_H
#define CPPTEST_QUICKSORT_USING_FUTURES_H

#include <algorithm>
#include <future>
#include <iostream>
#include <list>

namespace quicksort{
    template <typename T>
    std::list<T> quicksort_using_futures(std::list<T> input){
        if (input.empty()){
            return input;
        }
        std::list<T> result;
        result.splice(result.begin(), input, input.begin());
        T const &pivot = *result.begin();
        auto divide_point = std::partition(input.begin(), input.end(), [&](T const &t){return t < pivot;});
        std::list<T> lower_part;
        lower_part.splice(lower_part.end(), input, input.begin(), divide_point);
        std::future<std::list<T>> new_lower = std::async(quicksort_using_futures<T>, std::move(lower_part));

        std::list<T> new_higher = quicksort_using_futures<T>(std::move(input));

        result.splice(result.end(), new_higher);
        result.splice(result.begin(), new_lower.get());

        return result;

    }
}


#endif //CPPTEST_QUICKSORT_USING_FUTURES_H
