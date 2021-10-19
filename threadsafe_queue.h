//
// Created by yryang on 2021/10/19.
//

#ifndef CPPTEST_THREADSAFE_QUEUE_H
#define CPPTEST_THREADSAFE_QUEUE_H


#include <iostream>
#include <memory>
#include <queue>


namespace threadsafe_queue{
    template <typename T>
    class threadsafe_queue {
    private:
        std::queue<std::shared_ptr<T> > data_;
        mutable std::mutex mtx_;
        std::condition_variable data_cond_;

    public:
        threadsafe_queue(): data_(std::queue<std::shared_ptr<T>>()), mtx_(std::mutex()), data_cond_(std::condition_variable()){}
        threadsafe_queue(const threadsafe_queue& other) = delete;
        threadsafe_queue& operator=(const threadsafe_queue& other) = delete;

        void wait_and_pop(T &value){
            std::unique_lock<std::mutex> uniqueLock(mtx_);
            while (data_.empty()){
                data_cond_.wait(uniqueLock);
            }
            value = std::move(*data_.front());  //TODO 这里的std::move
            data_.pop();
        }
        bool try_pop(T &value){
            std::lock_guard<std::mutex> lockGuard(mtx_);
            if (data_.empty())
                return false;
            value = std::move(*data_.front());
            data_.pop();
            return true;
        }

        std::shared_ptr<T> wait_and_pop(){
            std::unique_lock<std::mutex> uniqueLock(mtx_);
            while (data_.empty()){
                data_cond_.wait(uniqueLock);
            }
            std::shared_ptr<T> res = data_.front();  //TODO 这里的std::move
            data_.pop();
            return res;
        }

        std::shared_ptr<T> try_pop(){
            std::lock_guard<std::mutex> lockGuard(mtx_);
            if (data_.empty())
                return std::shared_ptr<T>();
            std::shared_ptr<T> res = data_.front();
            data_.pop();
            return res;
        }

        void push(T new_value){
            std::shared_ptr<T> t = std::make_shared<T>(new_value);
            std::lock_guard<std::mutex> lockGuard(mtx_);
            data_.push(t);
            data_cond_.notify_all();
        }

        bool empty(){
            std::lock_guard<std::mutex> lockGuard(mtx_);
            return data_.empty();
        }

    };

    // 上面的queue使用了mutex，push或pop的时候都需要锁住mutex，那么能不能分割两者呢？
    // push使用head_mutex，pop使用tail_mutex，使用细粒度锁可以提高并行。
    template <typename T>
    class queue{
    private:
        struct node{
            std::shared_ptr<T> data;
            std::unique_ptr<node> next;
        };
        std::mutex head_mtx_;
        std::mutex tail_mtx_;
        std::unique_ptr<node> head_;
        node *tail_;
        std::condition_variable data_cond_;

        node* get_tail(){
            std::lock_guard<std::mutex> lockGuard(tail_mtx_);
            return tail_;
        }
        std::unique_ptr<node> pop_head(){
            std::unique_ptr<node> old_head = std::move(head_);
            head_ = std::move(old_head->next);
            return old_head;
        }

        std::unique_ptr<node> try_pop_head(){
            std::lock_guard<std::mutex> lockGuard(head_mtx_);
            if (head_.get() == get_tail()){
                return std::unique_ptr<node>();
            }
            return pop_head();
        }
        std::unique_ptr<node> try_pop_head(T &value){
            std::lock_guard<std::mutex> lockGuard(head_mtx_);
            if (head_.get() == get_tail()){
                return std::unique_ptr<node>();
            }
            value = std::move(*head_->data);
            return pop_head();
        }

    public:
        queue(): head_(new node), tail_(head_.get()){}
        queue(const queue &other) = delete;
        queue& operator=(const queue &other) = delete;



        void push(T new_value){
            std::shared_ptr<T> new_data = std::make_shared<T>(new_value);
            std::unique_ptr<node> tmp(new node);
            node* new_tail = tmp.get();
            {
                std::lock_guard<std::mutex> lockGuard(tail_mtx_);
                tail_->data = new_data;
                tail_->next = std::move(tmp);
                tail_ = new_tail;
            }
            data_cond_.notify_all();
        }

        std::shared_ptr<T> try_pop(){
            std::unique_ptr<node> old_head = try_pop_head();
            return old_head ? old_head->data : std::shared_ptr<T>();
        }

        bool try_pop(T &value){
            std::unique_ptr<node> old_head = try_pop_head(value);
            return old_head != nullptr;
        }


        std::shared_ptr<T> wait_and_pop(){
            std::unique_lock<std::mutex> uniqueLock(head_mtx_);
            while (head_.get() == get_tail()){
                data_cond_.wait(uniqueLock);
            }
            std::unique_ptr<node> old_head = pop_head();
            return old_head->data;
        }
        void wait_and_pop(T &value){
            std::unique_lock<std::mutex> uniqueLock(head_mtx_);
            while (head_.get() == get_tail()){
                data_cond_.wait(uniqueLock);
            }
            std::unique_ptr<node> old_head = pop_head();
            value = std::move(*old_head->data);
        }

        bool empty(){
            std::lock_guard<std::mutex> lockGuard(head_mtx_);
            return head_.get() == get_tail();
        }


    };
}



#endif //CPPTEST_THREADSAFE_QUEUE_H
