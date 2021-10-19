//
// Created by yryang on 2021/10/19.
//


#include <cassert>
#include <iostream>
#include <vector>
#include <thread>

#include "threadsafe_list.h"
#include "util.h"

namespace threadsafe_list{
    void push_test(threadsafe_list<int> &list, std::vector<int> v){
        for (auto &&vv: v){
            list.push_front(vv);
        }
    }
    void test(){
        threadsafe_list<int> list;
        int N = 30;
        int thread_num = 10;
        std::vector<std::thread> threads;
        for (int i = 0; i < thread_num; ++i) {
            std::vector<int> v;
            for (int j = 0; j < N/thread_num; ++j) {
                v.push_back(j + (N/thread_num)*i);
            }
            threads.emplace_back(push_test, std::ref(list), v);
        }
        for (int i = 0; i < thread_num; ++i) {
            threads[i].join();
        }


        std::cout << "begin for each:\n";
        std::vector<int> res;
        res.clear();
        list.for_each([&](int x){res.push_back(x); std::cout << x << std::endl;});


        list.remove_if([](int x){return x>4;});
        std::cout << "after remove x>4\n";
        list.for_each([](int x){std::cout << x << std::endl;});


        std::shared_ptr<int> sp = list.find_first_if([](int x){return x>3;});
        std::cout << "found first x>3: " << *sp << std::endl;

        std::sort(res.begin(), res.end());
        for (int i = 0; i < N; ++i) {
            ASSERT(res[i] == i, "thread safe list: wrong" );
        }
        std::cout << "test list success\n";

    }
}

int main(){
    threadsafe_list::test();
    return 0;
}