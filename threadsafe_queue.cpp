//
// Created by yryang on 2021/10/19.
//

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#include "threadsafe_queue.h"
#include "util.h"

namespace threadsafe_queue{

    template <class T>
    void push_test(T &queue, std::vector<int> v){
        for (auto &&vv: v){
            queue.push(vv);
        }


        v.clear();
        while (!queue.empty()){
            try {
                int t;
                bool flag = queue.try_pop(t);
                if (flag){
                    v.push_back(t);
                }

            } catch (...) {
                std::cout << "wrong empty\n";   // queue.empty() queue.pop() 有并发冲突
            }

        }
        for (auto &&vv: v){
            queue.push(vv);
        }
    }

    template <class T>
    void test(){
        T queue;
        int N = 1000;
        int thread_num = 10;
        std::vector<std::thread> threads;
        for (int i = 0; i < thread_num; ++i) {
            std::vector<int> v;
            for (int j = 0; j < N/thread_num; ++j) {
                v.push_back(j + (N/thread_num)*i);
            }
            threads.push_back(std::thread(push_test<T>, std::ref(queue), v));
        }
        for (int i = 0; i < thread_num; ++i) {
            threads[i].join();
        }
        std::vector<int> res;
        while (!queue.empty()){
            int t;
            queue.try_pop(t);
            res.push_back(t);
        }
        std::sort(res.begin(), res.end());
        for (int i = 0; i < N; ++i) {
            ASSERT(res[i] == i, "thread safe queue: wrong " );
        }
        std::cout << "test success\n";

    }

    template <class T>
    void test_wait_and_pop(){
        T queue;
        T queue_ans;
        int N = 100;
        int thread_num = 10;

        auto consumer = [](T &queue, T &queue_ans){
            while (true){
                int t;
                queue.wait_and_pop(t);
                std::cout << "thread_id :" << std::this_thread::get_id() <<" consume data " << t << std::endl;
                queue_ans.push(t);
            }

        };

        std::cout << "creating threads..." << std::endl;
        std::vector<std::thread> threads;
        threads.reserve(thread_num);
        for (int i = 0; i < thread_num; ++i) {
            threads.push_back(std::thread(consumer, std::ref(queue), std::ref(queue_ans)));
        }



        std::cout << "produce data...\n";
        for (int i = 0; i < N; ++i) {
            std::cout << "produce data " << i << std::endl;
            queue.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds (20));
        }
        std::cout << "producer done\n";

        std::vector<int> res;
        while (!queue_ans.empty()){
            int t;
            queue_ans.try_pop(t);
            std::cout << t << std::endl;
            res.push_back(t);
        }
        std::sort(res.begin(), res.end());
        for (int i = 0; i < N; ++i) {
            ASSERT(res[i] == i, "thread safe queue: wrong " );
        }
        std::cout << "test_wait_and_pop success\n";


    }
}

int main(){
//    threadsafe_queue::test<threadsafe_queue::threadsafe_queue<int> >();
    threadsafe_queue::test_wait_and_pop<threadsafe_queue::queue<int> >();
    return 0;
}