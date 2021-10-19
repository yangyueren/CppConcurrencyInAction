//
// Created by yryang on 2021/10/19.
//

#ifndef CPPTEST_THREADSAFE_LIST_H
#define CPPTEST_THREADSAFE_LIST_H

#include <iostream>



namespace threadsafe_list{
    template <typename T>
    class threadsafe_list {
    private:
        struct node{
            std::mutex mtx;
            std::shared_ptr<T> data;
            std::unique_ptr<node> next;
            node(): next(){
            }
            node(T const& value): data(std::make_shared<T>(value)){}
        };

        node head_; //dummpy head


    public:
        threadsafe_list(){}
        ~threadsafe_list(){
            remove_if<bool (node const&)>([](node const&){return true;});
        }

        void push_front(T const& value){
            std::unique_ptr<node> new_node(new node(value));
            std::lock_guard<std::mutex> lockGuard(head_.mtx);
            new_node->next = std::move(head_.next);
            head_.next = std::move(new_node);

        }

        template<typename Function>
        void for_each(Function f){
            node* current = &head_;
            std::unique_lock<std::mutex> lk(current->mtx);
            while (node* const next = current->next.get()){
                std::unique_lock<std::mutex> next_lk(next->mtx);
                lk.unlock();
                f(*next->data);
                current = next;
                lk = std::move(next_lk);
            }
        }

        template<typename Predicate>
        std::shared_ptr<T> find_first_if(Predicate p){
            node* current = &head_;
            std::unique_lock<std::mutex> lk(current->mtx);
            while (node* const next = current->next.get()){
                std::unique_lock<std::mutex> next_lk(next->mtx);
                lk.unlock();
                if(p(*next->data)){
                    return next->data;
                }
                current = next;
                lk = std::move(next_lk);
            }
            return std::shared_ptr<T>();
        }

        template<typename Predicate>
        void remove_if(Predicate p){
            node* current = &head_;
            std::unique_lock<std::mutex> lk(current->mtx);
            while (node* const next = current->next.get()){
                std::unique_lock<std::mutex> next_lk(next->mtx);

                if(p(*next->data)){
                    std::unique_ptr<node> old_next = std::move(current->next);
                    current->next = std::move(next->next);
                    next_lk.unlock();
                }else{
                    lk.unlock();
                    current = next;
                    lk = std::move(next_lk);
                }

            }
        }

        bool empty(){
            std::lock_guard<std::mutex> lockGuard(head_.mtx);
            return head_.next == nullptr;
        }

    };
}



#endif //CPPTEST_THREADSAFE_LIST_H
