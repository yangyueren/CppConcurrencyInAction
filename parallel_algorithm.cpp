//
// Created by yryang on 2021/10/21.
//

#include "parallel_algorithm.h"
//
// Created by yryang on 2021/10/21.
//

#include "parallel_algorithm.h"



#include <iostream>
#include <vector>
#include <chrono>
#include "util.h"


namespace parallel_algorithm{
    void for_each_test(){
        int N = 100;
        std::vector<int> v;
        v.reserve(N);
        for (int i = 0; i <N; ++i) {
            v.push_back(i);
        }
        auto f = [](int &x){
            x += 3;
        };
        std::vector<int> t = v;
        std::vector<int> v2 = v;

        parallel_for_each(v.begin(), v.end(), f);
        parallel_for_each_async(v2.begin(), v2.end(), f);
        std::for_each(t.begin(), t.end(), f);
        for (int i = 0; i < N; ++i) {
            std::cout << v[i] << " ";
            ASSERT(v[i] == t[i], "wrong ans.\n");
            ASSERT(v2[i] == t[i], "wrong ans.\n");
        }

        std::cout << "\nparallel exception safe accumulate test success\n";

    }


    void find_test(){
        int N = 100;
        std::vector<int> v;
        v.reserve(N);
        for (int i = 0; i <N; ++i) {
            v.push_back(i);
        }
        std::vector<int> t = v;
        std::vector<int> v2 = v;

//        auto i1 = parallel_find(v.begin(), v.end(), N/2);
        auto i2 = parallel_find_async(v.begin(), v.end(), N/2);
        auto i0 = std::find(v.begin(), v.end(), N/2);

//        ASSERT(i1 == i0, "wrong ans.\n");
        ASSERT(i2 == i0, "wrong ans.\n");


        std::cout << "\nparallel find test success\n";

    }


}

int main(){
//    parallel_algorithm::for_each_test();
    parallel_algorithm::find_test();
    return 0;
}