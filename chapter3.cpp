#include <iostream>
#include <thread>

//不要把保护的数据传出去引用或指针
class some_data{
private:
    int a;
    std::string b;
public:
    void do_something();
};

class data_wrapper{
private:
    some_data data;
    std::mutex m;
public:
    template<typename Func>
    void  process_data(Func f){
        std::scoped_lock g(m);
        f(data);
    }
};
some_data* unprotected;
void malicious_func(some_data& data){
    unprotected = &data;
}

data_wrapper w;
void f(){
    w.process_data(malicious_func);
    unprotected->do_something();
}

