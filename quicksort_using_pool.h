//
// Created by yryang on 2021/10/21.
//

#ifndef CPPTEST_QUICKSORT_USING_POOL_H
#define CPPTEST_QUICKSORT_USING_POOL_H


#include <algorithm>
#include <atomic>
#include <future>
#include <iostream>
#include <list>
#include <thread>
#include <vector>
#include <string>

#include "thread_pool_balanced_job.h"

namespace quicksort{

    template <typename T>
    class sorter_pool{
    private:

        thread_pool_balanced_job::thread_pool_balanced_job pool;

    public:

        sorter_pool(){}

        std::list<T> do_sort(std::list<T> &chunk_data){
            if (chunk_data.empty()){
                return chunk_data;
            }

            std::list<T> result;
            result.splice(result.begin(), chunk_data, chunk_data.begin());
            T const &pivot = *result.begin();
            auto divide_point = std::partition(chunk_data.begin(), chunk_data.end(), [&](T const &t){return t < pivot;});

            std::list<T> lower_part;
            lower_part.splice(lower_part.end(), chunk_data, chunk_data.begin(), divide_point);
            std::future<std::list<T> > new_lower = pool.submit(std::bind(&sorter_pool::do_sort, this, std::move(lower_part)));


            std::list<T> new_higher(do_sort(chunk_data));

            result.splice(result.end(), new_higher);

            while(new_lower.wait_for(std::chrono::seconds(0)) !=
                  std::future_status::ready) {
                pool.run_pending_task();
            }

            result.splice(result.begin(), new_lower.get());
            return result;

        }
    };


    template <typename T>
    std::list<T> quicksort_using_pool(std::list<T> input){
        if (input.empty()){
            return input;
        }
        sorter_pool<T> s;
        return s.do_sort(input);

    }
}


#endif //CPPTEST_QUICKSORT_USING_POOL_H
