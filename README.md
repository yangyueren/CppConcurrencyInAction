# CppConcurrencyInAction
The repository contains source code for the book CppConcurrencyInAction.
My notes are [here](https://www.notion.so/1a56bd8aaa934ad0a56c617302d03ffe?v=95b5115bbd764c0097a29941ff2d334b)

There are some basic thread safe data structures in first part, such as threadsafe_stack, threadsafe_queue.

There is a bug of threadsafe_lookup_table in the book. 
I fix it in the code with adding an overload function 
for `find_entry_for` with `const_iterator`.
```c++

// overload for const_iterator
bucket_const_iterator find_entry_for(Key const& key) const {
    return std::find_if(data.begin(), data.end(), [&key](bucket_value const& item){ return item.first == key;});
}

``` 

<br/>

As for std::packaged_task, we should provide it with function signature, but 
std::packaged_task<FunctionType> doesn't work.
I found a useful stackoverflow link which solved this problem.

```c++

template<typename FunctionType>
std::future<typename std::result_of<FunctionType()>::type>  submit(FunctionType f){
    typedef typename std::result_of<FunctionType()>::type result_type;

    std::packaged_task<result_type()> task(f);     // TODO, must use result_type(Args), or it will not work
    // https://stackoverflow.com/questions/22709480/template-with-packaged-task-not-compile

    std::future<result_type> res(task.get_future());
    work_queue.push(std::move(task));
    return res;
}

```

<br/>

`function_wrapper` is very useful to wrap some uncopyable object.
```c++

class function_wrapper{
    struct impl_base{
        virtual void call() = 0;
        virtual ~impl_base(){}
    };


    template<typename F>
    struct impl_type: impl_base{
        F f;
        impl_type(F &&f_): f(std::move(f_)){}
        void  call() override {f();}
    };

    std::unique_ptr<impl_base> impl;

public:
    template<typename F>
    function_wrapper(F &&f): impl(new impl_type<F>(std::move(f))){}

    void operator()(){
        impl->call();
    }
    function_wrapper() = default;
    function_wrapper(function_wrapper &&other): impl(std::move(other.impl)){

    }
    function_wrapper& operator=(function_wrapper &&other){
        impl = std::move(other.impl);
        return *this;
    }
    function_wrapper(const function_wrapper&) = delete;
    function_wrapper(function_wrapper&) = delete;
    function_wrapper& operator=(const function_wrapper&) = delete;
};


```


## BUGs
Some unexpected problems have appeared in parallel_quicksort:
You can reproduce the problem by running `quicksort_test.cpp` with command:
<br><br>
```bash
# cd /path/to/project
g++ quicksort_test.cpp -std=c++17 -I . -lpthread
./a.out
```
<br/>

When I set `int N = 10000;` to a larger value, eg `N = 20000`, 
the algorithm `quicksort_using_chunk` and `quicksort_using_pool`
will crash down. Perhaps it's due to that the process reached the 
maximum physical memory.



<br><br><br>
## TODO LIST
- [x] basic thread safe structures
- [x] concurrent algorithms
- [x] thread pool with three variants
