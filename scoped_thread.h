//
// Created by yryang on 2021/10/19.
//

#ifndef CPPTEST_SCOPED_THREAD_H
#define CPPTEST_SCOPED_THREAD_H

#include <iostream>
#include <thread>

namespace thread_guard{
    void func(){
        std::cout << "func" << std::endl;
    }

    class thread_guard{
        std::thread &t;
    public:
        explicit thread_guard(std::thread& t): t(t){

        }
        ~thread_guard(){
            if(t.joinable()){
                t.join();
            }
        }
        thread_guard(const thread_guard& t) = delete;
        thread_guard& operator=(const thread_guard&) = delete;

    };

    void test(){
        int local_state = 0;
        std::thread t(func);
        thread_guard g(t);
        std::cout << "do_some_thing_in_current_thread()" << std::endl;
    }


    class widget{

    };
    void update_data(widget& data){
        //
    }
    void test2(){
        widget data;
        std::thread t(update_data, std::ref(data)); // 如果不加 std::ref(data)编译不通过，
        // 尽管update_data的第一个参数期待传入一个引用，但是std::thread的构造函数并不知晓；
        //构造函数无视函数期待的参数类型，并盲目地拷贝已提供的变量。
    }

}

namespace scoped_thread{
    void func(int &x){
        std::cout << "func x=" << x << std::endl;
    }

    //scoped_thread 要比thread_guard更好
    class scoped_thread{
    private:
        std::thread t_;
    public:
        scoped_thread(std::thread &&t): t_(std::move(t)){
            if(!t_.joinable()){
                throw std::logic_error("No thread");
            }
        }
        ~scoped_thread(){
            t_.join();
        }
        scoped_thread(const scoped_thread&) = delete;
        scoped_thread& operator=(const scoped_thread&) = delete;
    };

    void test(){
        int some_local_state = 0;
        std::thread t(func, std::ref(some_local_state));
        scoped_thread s(std::move(t));
        std::cout << "do_some_thing_in_current_thread()\n";
    }
}

#endif //CPPTEST_SCOPED_THREAD_H
