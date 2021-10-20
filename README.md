# CppConcurrencyInAction
The repository contains source code for the book CppConcurrencyInAction.

There are some basic thread safe data structures in first part, such as threadsafe_stack, threadsafe_queue.

There is a bug of threadsafe_lookup_table in the book. I fix it in the code with adding an overload functin for `find_entry_for` with `const_iterator`.
```c++

// overload for const_iterator
bucket_const_iterator find_entry_for(Key const& key) const {
    return std::find_if(data.begin(), data.end(), [&key](bucket_value const& item){ return item.first == key;});
}

``` 


<br><br><br>
## TODO LIST
- [x] basic thread safe structures
- [ ] concurrent algorithms
- [ ] thread pool
