#include <utility>

#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server) {

    map<set<string>, int> document_to_word_freqs_;
    vector<int> id_del;
    id_del.reserve(search_server.document_ids_.size()-1);
    int n;

    for(int i = 0; i < search_server.document_ids_.size(); ++i){
        auto it = search_server.word_to_document_freqs_.begin();
        set<string> stmp;
        for(; it != search_server.word_to_document_freqs_.end(); ++it){
           if(it->second.count(search_server.document_ids_[i])){
               stmp.insert(it->first);
           }
        }

        n = document_to_word_freqs_.size();
        document_to_word_freqs_.insert(pair<set<string>, int>(stmp, search_server.document_ids_[i]));
        if(document_to_word_freqs_.size() == n){
          id_del.push_back(search_server.document_ids_[i]);
        }
    }

    for(int i = 0; i<id_del.size(); ++i){
        cout << "Found duplicate document id " << id_del[i] << endl;
        search_server.RemoveDocument(id_del[i]);
    }
}
