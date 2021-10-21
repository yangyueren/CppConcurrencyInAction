//
// Created by yryang on 2021/10/20.
//

#ifndef CPPTEST_THREAD_POOL_BALANCED_JOB_H
#define CPPTEST_THREAD_POOL_BALANCED_JOB_H


#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <iostream>

#include "function_wrapper.h"
#include "threadsafe_queue.h"
#include "scoped_thread.h"


namespace thread_pool_balanced_job{

    class work_stealing_queue
    {
    private:
        typedef function_wrapper data_type;
        std::deque<data_type> the_queue;  // 1
        mutable std::mutex the_mutex;

    public:
        work_stealing_queue()
        {}
        work_stealing_queue(const work_stealing_queue& other)=delete;
        work_stealing_queue& operator=(
                const work_stealing_queue& other)=delete;

        void push(data_type data)  // 2
        {
            std::lock_guard<std::mutex> lock(the_mutex);
            the_queue.push_front(std::move(data));
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> lock(the_mutex);
            return the_queue.empty();
        }

        bool try_pop(data_type& res)  // 3
        {
            std::lock_guard<std::mutex> lock(the_mutex);
            if(the_queue.empty()) {
                return false;
            }
            res=std::move(the_queue.front());
            the_queue.pop_front();
            return true;
        }

        bool try_steal(data_type& res)  // 4
        {
            std::lock_guard<std::mutex> lock(the_mutex);
            if(the_queue.empty()) {
                return false;
            }
            res=std::move(the_queue.back());
            the_queue.pop_back();
            return true;
        }
    };

    class thread_pool_balanced_job {
    private:
        typedef function_wrapper task_type;

        std::atomic_bool done;
        threadsafe_queue::queue<task_type> pool_work_queue;
        std::vector<std::unique_ptr<work_stealing_queue> > queues;
        std::vector<std::thread> threads;
        join_threads::join_threads joiner;

        static thread_local work_stealing_queue* local_work_queue;
        static thread_local unsigned int my_index_;

        std::mutex mtx;


        void worker_thread(unsigned int index){
            // TODO: use mtx to synchronize all threads
            // or some thread hasn't been created, the other thread wants to steal data from that thread,
            // program will crash.
            std::unique_lock<std::mutex> uniqueLock(mtx);
            uniqueLock.unlock();

            my_index_ = index;
            local_work_queue = queues[my_index_].get();
            while (!done){
                run_pending_task();
            }
        }

        static bool pop_from_local_queue(task_type &task){
            return local_work_queue && local_work_queue->try_pop(task);
        }
        bool pop_from_pool_queue(task_type &task){
            return pool_work_queue.try_pop(task);
        }
        bool pop_from_other_local_queue(task_type &task){
            for (int i = 0; i < queues.size(); ++i) {
                const unsigned int j = (my_index_+i+1) % queues.size();
//                std::cout << j << std::endl;
                if(queues[j]->try_steal(task)){
                    return true;
                }
            }
            return false;
        }



    public:
        thread_pool_balanced_job(): done(false), joiner(threads){
            try {
                std::lock_guard<std::mutex> lockGuard(mtx);
                const unsigned int thread_count = std::thread::hardware_concurrency();
                std::cout << thread_count << std::endl;
                for (int i = 0; i < thread_count; ++i) {
                    threads.emplace_back(&thread_pool_balanced_job::worker_thread, this, i); // this is a pointer
                    queues.push_back(std::make_unique<work_stealing_queue>());
                }
            } catch (...) {
                done = true;
                throw ;
            }
        }

        ~thread_pool_balanced_job(){
            done = true; // TODO, when done is set to true,
            // the worker thread will exit, even there are still tasks in work_queue.
        }

        void run_pending_task(){
            task_type task;
            if (pop_from_local_queue(task) || pop_from_pool_queue(task) || pop_from_other_local_queue(task)){
                task();
            }else{
                std::this_thread::yield();
            }
        }

        template<typename FunctionType>
        std::future<typename std::result_of<FunctionType()>::type>  submit(FunctionType f){
            typedef typename std::result_of<FunctionType()>::type result_type;

            std::packaged_task<result_type()> task(f);     // TODO, must use result_type(), or it will not work
            // https://stackoverflow.com/questions/22709480/template-with-packaged-task-not-compile

            std::future<result_type> res(task.get_future());
            if (local_work_queue){
                local_work_queue->push(std::move(task));
            }else{
                pool_work_queue.push(std::move(task));
            }
            return res;
        }
    };
}



#endif //CPPTEST_THREAD_POOL_BALANCED_JOB_H
