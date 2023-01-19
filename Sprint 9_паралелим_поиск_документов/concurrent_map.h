#pragma once
#include <algorithm>
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <mutex>
 
 
using namespace std::string_literals;
using namespace std;
 
template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);
 
    struct Access {
        public:
            lock_guard<mutex> guard;
            Value& ref_to_value;
            
            Access(const Key& key, mutex& m, map<Key, Value>& map_dic)
            : guard(m)
            , ref_to_value(map_dic[key]) {
        }
    };
 
    explicit ConcurrentMap(size_t bucket_count): dictionary_(bucket_count),
                                                 m_(bucket_count){}
    Access operator[](const Key& key){
        uint64_t index = (uint64_t)key%dictionary_.size();
        return {key, m_[index], dictionary_[index]};
    };
 
 
    std::map<Key, Value> BuildOrdinaryMap(){
        map<Key, Value> all_dictionary_;
        for(size_t i = 0;  i < dictionary_.size(); ++i){
            lock_guard<mutex> guard(m_[i]);
            for(auto it = dictionary_[i].begin(); it != dictionary_[i].end(); ++it){
                all_dictionary_[it->first] = it ->second;
            }
        } 
        return all_dictionary_;
    };
    
    void erase(int document_id){
        uint64_t index = (uint64_t)document_id%dictionary_.size();
        dictionary_[index].erase(document_id); 
    };
 
 
private:
    vector<map<Key, Value>> dictionary_{};
    vector<mutex> m_{};
};
 
