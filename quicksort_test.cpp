//
// Created by yryang on 2021/10/21.
//

#include "quicksort_sequential.h"
#include "quicksort_using_futures.h"
#include "quicksort_using_chunk.h"
#include "quicksort_using_pool.h"

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <functional>
#include <random>
#include <vector>
#include "util.h"

namespace quicksort{
    void test(std::function<std::list<int>(std::list<int>) > f){
        int N = 10000;
        std::vector<int> v;
        for (int i = 0; i < N; ++i) {
            v.push_back(i);
        }
        auto rng = std::default_random_engine {};
        std::shuffle(std::begin(v), std::end(v), rng);
        std::list<int> input;
        for(auto &&t: v){
            input.push_back(t);
        }

        auto start = std::chrono::system_clock::now();
        std::list<int> ans = f(input);
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "Cost time: " << std::setw(9) << diff.count() << " s\n";
        int i = 0;
        for (auto &&it : ans){
            ASSERT(it == i, "wrong");
            i += 1;
        }
        std::cout << "test success\n";
    }
}


// TODO, static member initialization must be contained in .cc file.
thread_local thread_pool_balanced_job::work_stealing_queue* thread_pool_balanced_job::thread_pool_balanced_job::local_work_queue = nullptr;
thread_local unsigned int thread_pool_balanced_job::thread_pool_balanced_job::my_index_ = -1;


void test1(){
    std::function<std::list<int>(std::list<int>) > f(quicksort::quicksort_sequential<int>);
    std::cout << "begin test sequential quicksort.\n";
    quicksort::test(f);
}

void test2(){
    std::function<std::list<int>(std::list<int>) > f_using_futures(quicksort::quicksort_using_futures<int>);
    std::cout << "begin test parallel quicksort using async and futures.\n";
    quicksort::test(f_using_futures);
}

void test3(){
    std::function<std::list<int>(std::list<int>) > f_using_chunk(quicksort::quicksort_using_chunk<int>);
    std::cout << "begin test parallel quicksort using promise and chunk.\n";
    quicksort::test(f_using_chunk);
}

void test4(){
    std::function<std::list<int>(std::list<int>) > f_using_pool(quicksort::quicksort_using_pool<int>);

    std::cout << "begin test parallel quicksort using thread pool.\n";
    quicksort::test(f_using_pool);
}
int main(){
    test1();
    test2();
    test3();
    test4();

    return 0;
}