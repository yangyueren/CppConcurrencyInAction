//
// Created by yryang on 2021/10/21.
//

#ifndef CPPTEST_EXCEPTION_SAFE_ACCUMULATE_H
#define CPPTEST_EXCEPTION_SAFE_ACCUMULATE_H


#include <algorithm>
#include <future>
#include <memory>
#include <mutex>
#include <numeric>
#include <vector>
#include <thread>

#include "scoped_thread.h"

namespace exception_safe_accumulate{
    template <typename Iterator, typename T>
    struct accumulate_block{
        T operator()(Iterator first, Iterator last){
            return std::accumulate(first, last, T());
        }
    };

    template <typename Iterator, typename T>
    T exception_safe_accumulate (Iterator first, Iterator last, T init){
        unsigned long const length = std::distance(first, last);

        if(!length){
            return init;
        }
        unsigned long const min_per_thread = 25;
        unsigned long const max_threads = (length + min_per_thread-1) / min_per_thread;
        unsigned long const hardward_threads_num = std::thread::hardware_concurrency();
        unsigned long const thread_num = std::min(hardward_threads_num!=0?hardward_threads_num:2, max_threads);
        unsigned long const block_size = length / thread_num;
        std::vector<std::future<T> > futures(thread_num - 1);
        std::vector<std::thread> threads(thread_num - 1);
        join_threads::join_threads joinThreads(threads);

        Iterator block_start = first;



        for (int i = 0; i < (thread_num-1); ++i) {
            Iterator block_end = block_start;
            std::advance(block_end, block_size);
            std::packaged_task<T(Iterator, Iterator)> task((accumulate_block<Iterator, T>()));
            futures[i] = task.get_future();
            threads[i] = std::thread(std::move(task), block_start, block_end);
            block_start = block_end;
        }
        T last_result = accumulate_block<Iterator, T>()(block_start, last);

        std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));


        T result = init;

        for (int i = 0; i < (thread_num - 1); ++i) {
            result += futures[i].get();
        }
        result += last_result;
        return result;


    };
}



#endif //CPPTEST_EXCEPTION_SAFE_ACCUMULATE_H
