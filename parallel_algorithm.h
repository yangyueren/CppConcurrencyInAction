//
// Created by yryang on 2021/10/21.
//

#ifndef CPPTEST_PARALLEL_ALGORITHM_H
#define CPPTEST_PARALLEL_ALGORITHM_H

#include <algorithm>
#include <future>
#include <memory>
#include <mutex>
#include <numeric>
#include <vector>
#include <thread>

#include "scoped_thread.h"

namespace parallel_algorithm{
    template <typename Iterator, typename Func>
    void parallel_for_each(Iterator first, Iterator last, Func f){
        unsigned long const length = std::distance(first, last);

        if(!length){
            return;
        }
        unsigned long const min_per_thread = 25;
        unsigned long const max_threads = (length + min_per_thread-1) / min_per_thread;
        unsigned long const hardward_threads_num = std::thread::hardware_concurrency();
        unsigned long const thread_num = std::min(hardward_threads_num!=0?hardward_threads_num:2, max_threads);
        unsigned long const block_size = length / thread_num;

        std::vector<std::future<void> > futures(thread_num - 1);
        std::vector<std::thread> threads(thread_num - 1);
        join_threads::join_threads joinThreads(threads);

        Iterator block_start = first;
        for (int i = 0; i < thread_num - 1; ++i) {
            Iterator block_end = block_start;
            std::advance(block_end, block_size);
            std::packaged_task<void(void)> task(
                    [=]{
                        std::for_each(block_start, block_end, f);
                    }
                    );
            futures[i] = task.get_future();
            threads[i] = std::thread(std::move(task));
            block_start = block_end;
        }
        std::for_each(block_start, last, f);
        for (int i = 0; i < thread_num - 1; ++i) {
            futures[i].get();
        }

    }

    template <typename Iterator, typename Func>
    void parallel_for_each_async(Iterator first, Iterator last, Func f){
        unsigned long const length = std::distance(first, last);

        if(!length){
            return;
        }
        unsigned long const min_per_thread = 25;
        unsigned long const max_threads = (length + min_per_thread-1) / min_per_thread;
        unsigned long const hardward_threads_num = std::thread::hardware_concurrency();
        unsigned long const thread_num = std::min(hardward_threads_num!=0?hardward_threads_num:2, max_threads);
        unsigned long const block_size = length / thread_num;

        std::vector<std::future<void> > futures(thread_num - 1);
        std::vector<std::thread> threads(thread_num - 1);
        join_threads::join_threads joinThreads(threads);

        Iterator block_start = first;
        for (int i = 0; i < thread_num - 1; ++i) {
            Iterator block_end = block_start;
            std::advance(block_end, block_size);
            auto fn = [=]{
                        std::for_each(block_start, block_end, f);
                    };
            futures[i] = std::async(fn);
            block_start = block_end;
        }
        std::for_each(block_start, last, f);
        for (int i = 0; i < thread_num - 1; ++i) {
            futures[i].get();
        }
    }

    /*
     * It’s a particular example of how data access patterns can affect the design of your code (section 8.3.2)
     * For algorithms such as std::find the ability to complete “early” is an important property and not something
     * to squander. You therefore need to design your code to make use of it—to interrupt the other tasks in some way
     * when the answer is known, so that the code doesn’t have to wait for the other worker threads to process
     * the remaining elements.
     *
     * One way in which you can interrupt the other threads is by making use of an atomic variable as a flag
     * and checking the flag after processing every element.
     *
     * Now you have two choices as to how to return the values and how to propagate any exceptions. You can use an
     * array of futures, use std::packaged_task for transferring the values and exceptions, and then process the
     * results back in the main thread; or you can use std::promise to set the final result directly from the worker
     * threads. It all depends on how you wish to handle exceptions from the worker threads. If you want to stop on
     * the first exception (even if you haven’t processed all elements), you can use std::promise to set both the value
     * and the exception. On the other hand, if you want to allow the other workers to keep searching, you can
     * use std::packaged_task, store all the exceptions, and then rethrow one of them if a match isn’t found.
     */
    template <typename Iterator, typename MatchType>
    Iterator parallel_find(Iterator first, Iterator last, MatchType match){

        struct find_element{
            void operator()(Iterator begin, Iterator end, MatchType match, std::promise<Iterator>* result,
                    std::atomic<bool>* done_flag){  // TODO? why not use std::atomic_bool
                try {
                    for(; (begin!=end) && !done_flag->load(); ++begin ){
                        if(*begin == match){
                            result->set_value(begin);
                            done_flag->store(true);
                            return;
                        }
                    }
                } catch (...) {
                    try {
                        result->set_exception(std::current_exception());
                        done_flag->store(true);
                        /*
                         * If an exception is thrown, this is caught by the catchall handler f , and you try to store
                         * the exception in the promise g before setting the done_flag. Setting the value on the promise
                         * might throw an exception if the promise is already set, so you catch and discard any
                         * exceptions that happen here
                         */
                    } catch (...) {

                    }
                }
            }
        };


        unsigned long const length = std::distance(first, last);

        if(!length){
            return last;
        }
        unsigned long const min_per_thread = 25;
        unsigned long const max_threads = (length + min_per_thread-1) / min_per_thread;
        unsigned long const hardward_threads_num = std::thread::hardware_concurrency();
        unsigned long const thread_num = std::min(hardward_threads_num!=0?hardward_threads_num:2, max_threads);
        unsigned long const block_size = length / thread_num;

        std::vector<std::future<void> > futures(thread_num - 1);
        std::vector<std::thread> threads(thread_num - 1);


        std::promise<Iterator> result;
        std::atomic<bool> done_flag(false);
        {
            join_threads::join_threads joinThreads(threads);
            Iterator block_start = first;
            for (int i = 0; i < thread_num - 1; ++i) {
                Iterator block_end = block_start;
                std::advance(block_end, block_size);

                threads[i] = std::thread(find_element(), block_start, block_end, match, &result, &done_flag);
                block_start = block_end;
            }
            find_element()(block_start, last, match, &result, &done_flag);
        }
        if (done_flag.load()){
            return result.get_future().get();
        }else{
            return last;
        }
    }

    template <typename Iterator, typename MatchType>
    Iterator parallel_find_async_impl(Iterator first, Iterator last, MatchType match, std::atomic<bool> &done){
        try {
            unsigned long const length = std::distance(first, last);
            unsigned long const min_per_thread = 25;
            if (length < 2*min_per_thread){
                for(; first!=last && !done.load(); first++){
                    if(*first==match){
                        done.store(true);
                        return first;
                    }
                }
                return last;
            }else{
                Iterator const mid_point = first + (length/2);
                std::future<Iterator> right_future = std::async(&parallel_find_async_impl<Iterator, MatchType>,
                        mid_point, last, match, std::ref(done));
                Iterator const left_result = parallel_find_async_impl(first, mid_point, match, done);
                return (left_result == mid_point) ? right_future.get() : left_result;
            }

        } catch (...) {
            done.store(true);
            throw;
        }
    }
    template <typename Iterator, typename MatchType>
    Iterator parallel_find_async(Iterator first, Iterator last, MatchType match){
        std::atomic<bool> done(false);
        return parallel_find_async_impl(first, last, match, done);
    }


    template<typename Iterator>
    void parallel_partial_sum(Iterator first,Iterator last){

    }


    template<typename Iterator>
    void parallel_partial_sum_barrier(Iterator first,Iterator last){

    }


}



#endif //CPPTEST_PARALLEL_ALGORITHM_H
