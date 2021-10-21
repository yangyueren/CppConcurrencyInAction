//
// Created by yryang on 2021/10/19.
//

#ifndef CPPTEST_THREADSAFE_STACK_H
#define CPPTEST_THREADSAFE_STACK_H

#include <iostream>
#include <thread>
#include <stack>

namespace threadsafe_stack{
    struct empty_stack: std::exception{
        const char* what() const throw(){
            return "empty stack";
        }
    };
    template <typename T>
    class threadsafe_stack{
    private:
        std::stack<T> data_;
        mutable std::mutex mtx_;

    public:
        threadsafe_stack(): data_(std::stack<T>()){}

        threadsafe_stack(const threadsafe_stack& other){
            std::lock_guard<std::mutex> lockGuard(other.mtx_);
            data_ = other.data_;
        }

        threadsafe_stack& operator=(const threadsafe_stack&) = delete;

        void push(T const &new_value){
            std::lock_guard<std::mutex> lockGuard(mtx_);
            data_.push(new_value);
        }

        void push(T &&new_value){
            std::lock_guard<std::mutex> lockGuard(mtx_);
            data_.push(std::move(new_value));
        }

        std::shared_ptr<T> pop(){
            std::lock_guard<std::mutex> lockGuard(mtx_);
            if(data_.empty()) return std::shared_ptr<T>();

            std::shared_ptr<T> const res(std::make_shared<T>(std::move(data_.top())));
            data_.pop();
            return res;
        }

        bool pop(T &value){
            std::lock_guard<std::mutex> lockGuard(mtx_);
            if (data_.empty()) return false;

            value = std::move(data_.top());
            data_.pop();
            return true;
        }

        bool empty() const{
            std::lock_guard<std::mutex> lockGuard(mtx_);
            return data_.empty();
        }
    };

}






#endif //CPPTEST_THREADSAFE_STACK_H
