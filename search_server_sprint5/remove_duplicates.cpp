#include <utility>

#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {
    
    map<int, set<string>> document_to_word_freqs_;
    auto it = search_server.word_to_document_freqs_.begin();
    for(; it != search_server.word_to_document_freqs_.end(); ++it){
        auto it2 = it->second.begin();
        for(; it2 != it->second.end(); ++it2){
            document_to_word_freqs_[it2->first].insert(it->first);
        }
    } 
    
    multimap<set<string>,int> mm;
    auto it1 = document_to_word_freqs_.begin();
    for(; it1 != document_to_word_freqs_.end(); ++it1){
        mm.insert(pair<set<string>,int>(it1->second, it1->first));
    }
    
    vector<int> id_del;
    id_del.reserve(mm.size());
    auto prev = mm.begin();
    auto it3 = mm.begin();
    it3++;
    
    for( ; it3!= mm.end(); it3++, prev++){
        if(it3->first == prev->first){
            id_del.push_back(it3->second);
        }
    }
    
    for(int i = 0; i<id_del.size(); ++i){
        cout << "Found duplicate document id " << id_del[i] << endl;
        search_server.RemoveDocument(id_del[i]);
    }
}