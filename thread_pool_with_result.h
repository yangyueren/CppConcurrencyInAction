//
// Created by yryang on 2021/10/20.
//

#ifndef CPPTEST_THREAD_POOL_WITH_RESULT_H
#define CPPTEST_THREAD_POOL_WITH_RESULT_H

#include <atomic>
#include <future>
#include <memory>
#include <iostream>

#include "function_wrapper.h"
#include "threadsafe_queue.h"
#include "scoped_thread.h"


// use packaged_task to return future
namespace thread_pool_with_result{



    class thread_pool_with_result {
    private:
        std::atomic_bool done;
        threadsafe_queue::queue<function_wrapper> work_queue;
        std::vector<std::thread> threads;
        join_threads::join_threads joiner;

        void worker_thread(){
            while (!done){
                function_wrapper task;
                if(work_queue.try_pop(task)){
                    task();
                }else{
                    std::this_thread::yield();
                }
            }
        }

    public:
        thread_pool_with_result(): done(false), joiner(threads){
            try {
                const unsigned int thread_count = std::thread::hardware_concurrency();
                for (int i = 0; i < thread_count; ++i) {
                    threads.emplace_back(&thread_pool_with_result::worker_thread, this); // this is a pointer
                }
            } catch (...) {
                done = true;
                throw ;
            }
        }

        ~thread_pool_with_result(){
            done = true; // TODO, when done is set to true,
            // the worker thread will exit, even there are still tasks in work_queue.
        }

        template<typename FunctionType>
        std::future<typename std::result_of<FunctionType()>::type>  submit(FunctionType f){
            typedef typename std::result_of<FunctionType()>::type result_type;

            std::packaged_task<result_type()> task(f);     // TODO, must use result_type(Args), or it will not work
            // https://stackoverflow.com/questions/22709480/template-with-packaged-task-not-compile

            std::future<result_type> res(task.get_future());
            work_queue.push(std::move(task));
            return res;
        }

    };
}

/*
 *

https://stackoverflow.com/questions/22709480/template-with-packaged-task-not-compile

int func_string(const std::string &x)
{
    return 1;
}

int func_int(int x)
{
    return 2;
}

template <typename F, typename A>
std::future<typename std::result_of<F(A)>::type> spawn_task(F &&f, A &&a)
{
    typedef typename std::result_of<F(A)>::type result_type;
    std::packaged_task<result_type(A)> task(std::move(f));
    std::future<result_type> res(task.get_future());
    std::thread t(std::move(task), std::move(a));
    t.detach();
    return res;
}

int main()
{
    std::string str = "abc";

     auto res1 = spawn_task(func_string, str);
     res1.get();
    auto res2 = spawn_task(func_int, 10);
    res2.get();
}


 */


#endif //CPPTEST_THREAD_POOL_WITH_RESULT_H
