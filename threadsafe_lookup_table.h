//
// Created by yryang on 2021/10/19.
//

#ifndef CPPTEST_THREADSAFE_LOOKUP_TABLE_H
#define CPPTEST_THREADSAFE_LOOKUP_TABLE_H

#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <list>
#include <shared_mutex>

namespace threadsafe_lookup_table{
    template <typename Key, typename Value, typename Hash=std::hash<Key> >
    class threadsafe_lookup_table {
    private:
        class bucket_type{
        private:
            typedef std::pair<Key, Value> bucket_value;
            typedef std::list<bucket_value> bucket_data;
            typedef typename bucket_data ::iterator bucket_iterator;
            typedef typename bucket_data ::const_iterator bucket_const_iterator;

            bucket_data data;
            mutable std::shared_mutex mtx;

            bucket_iterator find_entry_for(Key const& key)  {
                return std::find_if(data.begin(), data.end(), [&key](bucket_value const& item){ return item.first == key;});
            }

            // overload for const_iterator
            bucket_const_iterator find_entry_for(Key const& key) const {
                return std::find_if(data.begin(), data.end(), [&key](bucket_value const& item){ return item.first == key;});
            }

        public:
            Value value_for(Key const &key, Value const &default_value) const{
                std::shared_lock<std::shared_mutex> sharedLock(mtx);
                bucket_const_iterator found_entry = find_entry_for(key);
                return (found_entry == data.end()) ? default_value : found_entry->second;

            }

            void add_or_update_mapping(Key const &key, Value const &value){
                std::unique_lock<std::shared_mutex> uniqueLock(mtx);
                bucket_iterator const found_entry = find_entry_for(key);
                if (found_entry == data.end()){
                    data.push_back(bucket_value(key, value));
                }else{
                    found_entry->second = value;
                }
            }

            void remove_mapping(Key const &key){
                std::unique_lock<std::shared_mutex> uniqueLock(mtx);
                bucket_iterator const found_entry = find_entry_for(key);
                if (found_entry != data.end()){
                    data.erase(found_entry);
                }
            }

            void get_map(std::map<Key, Value> &res) const{
                std::shared_lock<std::shared_mutex> sharedLock(mtx);
                for(auto &&it: data){
                    res.insert(it);
                }
            }
        }; // end of class bucket_type



        std::vector<std::unique_ptr<bucket_type> > buckets;
        Hash hasher;

        bucket_type& get_bucket(Key const &key) const{
            int idx = hasher(key) % buckets.size();
            return *buckets[idx];
        }

    public:
        threadsafe_lookup_table(unsigned num_buckets = 19,
                                Hash const& hasher_ = Hash()): buckets(num_buckets), hasher(hasher_){
            for (int i = 0; i < num_buckets; ++i) {
                buckets[i].reset(new bucket_type);
            }
        }
        threadsafe_lookup_table(threadsafe_lookup_table const &other) = delete;
        threadsafe_lookup_table& operator=(threadsafe_lookup_table const &other) = delete;


        Value value_for(Key const &key, Key const &default_value) const {
            return get_bucket(key).value_for(key, default_value);
        }

        void add_or_update_mapping(Key const &key, Value const &value){
            get_bucket(key).add_or_update_mapping(key, value);
        }

        void remove_mapping(Key const &key){
            get_bucket(key).remove_mapping(key);
        }

        std::map<Key, Value> get_map() const{
            std::map<Key, Value> res;
            for (int i = 0; i < buckets.size(); ++i) {
                buckets[i]->get_map(res);
            }
            return res;
        }
    };
} // end of namespace threadsafe_lookup_table

#endif //CPPTEST_THREADSAFE_LOOKUP_TABLE_H
