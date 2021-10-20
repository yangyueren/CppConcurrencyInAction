//
// Created by yryang on 2021/10/20.
//

#ifndef CPPTEST_THREAD_POOL_H
#define CPPTEST_THREAD_POOL_H

#include <iostream>
#include <atomic>
#include <thread>
#include <future>

#include "threadsafe_queue.h"
#include "scoped_thread.h"

namespace thread_pool{
    class thread_pool {

    private:

        // TODO the declaration order is important.
        std::atomic_bool done;
        threadsafe_queue::threadsafe_queue<std::function<void()> > work_queue;
        std::vector<std::thread> threads;
        join_threads::join_threads joiner;

        void worker_thread(){
            while (!done){
                std::function<void()> task;
                if (work_queue.try_pop(task)){
                    std::cout << "task\n";
                    task();
                }else{
//                    std::this_thread::sleep_for(std::chrono::seconds(1));
//                    std::cout << "yield\n";
                    std::this_thread::yield();

                }
            }
        }

    public:
        thread_pool(): done(false), joiner(threads){
            unsigned const thread_count = std::thread::hardware_concurrency();

            try {
                for (int i = 0; i < thread_count; ++i) {
                    threads.emplace_back(&thread_pool::worker_thread, this); // add work thread
                }
            } catch (...) {
                done = true;
                throw;
            }
        }

        ~thread_pool(){
            done = true; // TODO, when done is set to true,
                         // the worker thread will exit, even there are still tasks in work_queue.
        }

        void submit(void(*f)()){
            work_queue.push(std::function<void()>(f));
        }

    };

}


#endif //CPPTEST_THREAD_POOL_H
