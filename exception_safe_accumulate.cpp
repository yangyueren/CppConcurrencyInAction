//
// Created by yryang on 2021/10/21.
//

#include "exception_safe_accumulate.h"



#include <iostream>
#include <vector>
#include <chrono>
#include "util.h"


namespace exception_safe_accumulate{
    void test(){
        int N = 1000;
        std::vector<int> v;
        v.reserve(N);
        for (int i = 0; i <N; ++i) {
            v.push_back(i);
        }
        std::vector<int> t = v;
        int res = exception_safe_accumulate<typename std::vector<int>::iterator , int>(v.begin(), v.end(), 0);
        int test_ans = std::accumulate(t.begin(), t.end(), 0);
        ASSERT(res == test_ans, "wrong ans.\n");
        std::cout << "parallel exception safe accumulate test success\n";

    }
}

int main(){
    exception_safe_accumulate::test();
    return 0;
}