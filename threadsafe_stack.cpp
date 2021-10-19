//
// Created by yryang on 2021/10/19.
//

#include <cassert>
#include <iostream>
#include <vector>


#include "threadsafe_stack.h"
#include "util.h"

namespace threadsafe_stack{
    void push_test(threadsafe_stack<int> &stack, std::vector<int> v){
        for (auto &&vv: v){
            stack.push(vv);
        }


        v.clear();
        while (!stack.empty()){
            try {
                int t;
                stack.pop(t);
                v.push_back(t);
            } catch (...) {
                std::cout << "wrong empty\n";   // stack.empty() stack.pop() 有并发冲突
            }

        }
        for (auto &&vv: v){
            stack.push(vv);
        }
    }
    void test(){
        threadsafe_stack<int> stack;
        int N = 1000;
        int thread_num = 10;
        std::vector<std::thread> threads;
        for (int i = 0; i < thread_num; ++i) {
            std::vector<int> v;
            for (int j = 0; j < N/thread_num; ++j) {
                v.push_back(j + (N/thread_num)*i);
            }
            threads.emplace_back(push_test, std::ref(stack), v);
        }
        for (int i = 0; i < thread_num; ++i) {
            threads[i].join();
        }
        std::vector<int> res;
        while (!stack.empty()){
            int t;
            stack.pop(t);
            res.push_back(t);
        }
        std::sort(res.begin(), res.end());
        for (int i = 0; i < N; ++i) {
            ASSERT(res[i] == i, "thread safe stack: wrong" );
        }
        std::cout << "test success\n";

    }
}

int main(){
    threadsafe_stack::test();
    return 0;
}