//
// Created by yryang on 2021/10/19.
//

#include "threadsafe_lookup_table.h"

#include <iostream>
#include <vector>
#include <thread>
#include "util.h"

namespace threadsafe_lookup_table{

    void push_test(threadsafe_lookup_table<int, int> &lb, std::vector<int> v){
        for (auto &&vv: v){
            lb.add_or_update_mapping(vv, vv*2);
        }

        for (auto &&vv: v){
            lb.value_for(vv, 0);
        }

        for (auto &&vv: v){
            lb.remove_mapping(vv);
        }
        for (auto &&vv: v){
            lb.add_or_update_mapping(vv, vv);
        }

    }

    void test(){
        threadsafe_lookup_table<int, int> lb;
        int N = 30;
        int thread_num = 10;
        std::vector<std::thread> threads;
        for (int i = 0; i < thread_num; ++i) {
            std::vector<int> v;
            for (int j = 0; j < N/thread_num; ++j) {
                v.push_back(j + (N/thread_num)*i);
            }
            threads.emplace_back(push_test, std::ref(lb), v);
        }
        for (int i = 0; i < thread_num; ++i) {
            threads[i].join();
        }

        std::map<int, int> res = lb.get_map();
        for (int i = 0; i < N; ++i) {
            ASSERT(res[i] == i, "thread safe look_up_table: wrong" );
        }
        std::cout << "test look_up_table success\n";
    }
}

int main(){
    threadsafe_lookup_table::test();
    return 0;
}