//
// Created by yryang on 2021/10/19.
//

#ifndef CPPTEST_UTIL_H
#define CPPTEST_UTIL_H

#ifndef NDEBUG
#   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif

#endif //CPPTEST_UTIL_H
