cmake_minimum_required(VERSION 3.15)
project(cpptest)

set(CMAKE_CXX_STANDARD 17)



add_executable(scoped_thread scoped_thread.cpp)
add_executable(threadsafe_stack threadsafe_stack.cpp)
add_executable(threadsafe_queue threadsafe_queue.cpp)
add_executable(threadsafe_list threadsafe_list.cpp)