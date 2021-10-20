//
// Created by yryang on 2021/10/20.
//

#include "thread_pool.h"


#include <iostream>
#include <vector>
#include <chrono>
#include "util.h"


namespace thread_pool{
    void func(){
//        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << &std::this_thread::get_id << std::endl;
    }
    void test(){
        thread_pool pool;
        int N = 1000;
        int task_num = 10;
        std::vector<std::thread> threads;
        for (int i = 0; i < task_num; ++i) {
            std::vector<int> v;
            for (int j = 0; j < N/task_num; ++j) {
                v.push_back(j + (N/task_num)*i);
            }
            pool.submit(&func);
        }
        std::this_thread::sleep_for(std::chrono::seconds(4));
        std::cout << "test success\n";

    }
}

int main(){
    thread_pool::test();
    return 0;
}