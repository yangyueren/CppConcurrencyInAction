# CppConcurrencyInAction
The repository contains source code for the book CppConcurrencyInAction.

There are some basic thread safe data structures in first part, such as threadsafe_stack, threadsafe_queue.

There is a bug of threadsafe_lookup_table in the book. 
I fix it in the code with adding an overload functin 
for `find_entry_for` with `const_iterator`.
```c++

// overload for const_iterator
bucket_const_iterator find_entry_for(Key const& key) const {
    return std::find_if(data.begin(), data.end(), [&key](bucket_value const& item){ return item.first == key;});
}

``` 

<br>
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

<br><br><br>
## TODO LIST
- [x] basic thread safe structures
- [ ] concurrent algorithms
- [x] thread pool
