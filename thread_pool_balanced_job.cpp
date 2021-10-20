//
// Created by yryang on 2021/10/20.
//

#include "thread_pool_balanced_job.h"


#include <iostream>
#include <vector>
#include <chrono>
#include "util.h"


namespace thread_pool_balanced_job{
    int func(){
//        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << &std::this_thread::get_id << std::endl;
        return 222;
    }

    struct functor{
        int a;
        int operator()(){
            return a;
        }
    };
    void test(){
        thread_pool_balanced_job pool;
        int N = 1000;
        int task_num = 10;
        std::vector<std::thread> threads;
        std::vector<std::future<int> > futures;
        for (int i = 0; i < task_num; ++i) {
            std::vector<int> v;
            for (int j = 0; j < N/task_num; ++j) {
                v.push_back(j + (N/task_num)*i);
            }
            functor f{i};
            futures.push_back(pool.submit(f));
        }
        std::this_thread::sleep_for(std::chrono::seconds(4));
        for (int i = 0; i < futures.size(); ++i) {
//            futures[i].get();
            int res = futures[i].get();
            std::cout << "res " << res << std::endl;
            ASSERT(res == i, "wrong");
        }
        std::cout << "test success\n";

    }
}

thread_local thread_pool_balanced_job::work_stealing_queue* thread_pool_balanced_job::thread_pool_balanced_job::local_work_queue = nullptr;
thread_local unsigned int thread_pool_balanced_job::thread_pool_balanced_job::my_index_ = -1;

int main(){

    thread_pool_balanced_job::test();
    return 0;
}