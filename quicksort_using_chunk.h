//
// Created by yryang on 2021/10/21.
//

#ifndef CPPTEST_QUICKSORT_USING_CHUNK_H
#define CPPTEST_QUICKSORT_USING_CHUNK_H



#include <algorithm>
#include <atomic>
#include <future>
#include <iostream>
#include <list>
#include <thread>
#include <vector>
#include <string>

#include "threadsafe_stack.h"
#include "threadsafe_queue.h"

namespace quicksort{

    template <typename T>
    class sorter{
    private:
        struct chunk{
             std::list<T> data;
             std::promise<std::list<T> > promise;
        };
        const int max_threads_num;
        std::vector<std::thread> threads;
        threadsafe_stack::threadsafe_stack<std::shared_ptr<chunk> > stack;

        std::atomic_bool done;

        std::mutex mtx;

    public:

        sorter(): max_threads_num(std::thread::hardware_concurrency()-1), done(false){}

        ~sorter(){
            done = true;
            for (int i = 0; i < threads.size(); ++i) {
                threads[i].join();
            }
        }

        void sort_chunk(std::shared_ptr<chunk> const &p){
            if (p->data.empty()){
                p->promise.set_value(std::list<T>());
                return;
            }
            p->promise.set_value(do_sort(p->data));
        }

        void try_sort_chunk(){
            std::shared_ptr<chunk> chunk_p;

            if (stack.pop(chunk_p)){
                sort_chunk(chunk_p);
            }
        }

        void sort_thread(){
            while (!done){
                try_sort_chunk();
                std::this_thread::yield();
            }
        }

        void debug(std::list<T> data, std::string des=""){
            std::lock_guard<std::mutex> lockGuard(mtx);
            std::cout << des << std::endl;
            for(auto i: data){
                std::cout << i << " ";
            }
            std::cout << '\n';
        }

        std::list<T> do_sort(std::list<T> &chunk_data){
            if (chunk_data.empty()){
                return chunk_data;
            }

            std::list<T> result;
            result.splice(result.begin(), chunk_data, chunk_data.begin());
            T const &pivot = *result.begin();
            auto divide_point = std::partition(chunk_data.begin(), chunk_data.end(), [&](T const &t){return t < pivot;});

            std::shared_ptr<chunk> lower_part = std::make_shared<chunk>();
            lower_part->data.splice(lower_part->data.end(), chunk_data, chunk_data.begin(), divide_point);
            std::future<std::list<T> > new_lower = lower_part->promise.get_future();

            stack.push(lower_part);


            if (threads.size() < max_threads_num){
                std::lock_guard<std::mutex> lockGuard(mtx);
                threads.emplace_back(&sorter<T>::sort_thread, this);
            }

            std::list<T> new_higher(do_sort(chunk_data));

            result.splice(result.end(), new_higher);

            while(new_lower.wait_for(std::chrono::seconds(0)) !=
                  std::future_status::ready) {
                try_sort_chunk();
            }

            result.splice(result.begin(), new_lower.get());
            return result;

        }
    };


    template <typename T>
    std::list<T> quicksort_using_chunk(std::list<T> input){
        if (input.empty()){
            return input;
        }
        sorter<T> s;
        return s.do_sort(input);

    }
}



#endif //CPPTEST_QUICKSORT_USING_CHUNK_H
