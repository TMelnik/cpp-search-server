#pragma once
#include <set>
#include <map>
#include <algorithm>
#include <execution>
#include <cstddef>
#include <functional>
#include <iostream>
#include <time.h>


#include "document.h"
#include "string_processing.h"
#include "concurrent_map.h"


const int MAX_RESULT_DOCUMENT_COUNT = 5;
const float EPS = 1e-6;

using namespace std;


class SearchServer {
    private:
    
    struct Query {
        vector<string_view> minus_words;
        vector<string_view> plus_words;
    };
    
    struct QueryWord {
        string_view data;
        bool is_minus;
        bool is_stop;
    };


public:
    map<string_view, map<int, double>> word_to_document_freqs_;
    map<int, map<string_view, double>> document_words_freqs_;
    set<int> document_ids_;
    
    void RemoveDocument(int document_id);
    
    template<typename ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&& policy, int document_id){
        if(!document_ids_.count(document_id)){
            return;
        }
        
        vector<string_view> words(document_words_freqs_.at(document_id).size());
        auto it = document_words_freqs_.at(document_id);
        transform(policy, it.begin(), it.end(), words.begin(), [](auto& pair) {
            return pair.first;
        });
        
        for_each(policy, words.begin(), words.end(), [this, document_id](auto word) {
            auto x = word_to_document_freqs_.find(word);
            x->second.erase(document_id);
          
        });
        documents_.erase(document_id);
        document_ids_.erase(document_id);
    }
    
    
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);
    
    explicit SearchServer(const string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text)){}

    explicit SearchServer(const string_view stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text)){}
    
    void AddDocument(int document_id, const string_view document, DocumentStatus status, const vector<int>& ratings);


    template <typename DocumentPredicate, typename ExecutionPolicy>
    vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query, DocumentPredicate document_predicate) const;
    
    template <typename ExecutionPolicy>
    vector<Document>FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query, DocumentStatus status) const;
    
    template <typename ExecutionPolicy>
    vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query) const;
    
    template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string_view raw_query, DocumentPredicate document_predicate) const;
    
    vector<Document>FindTopDocuments(const string_view raw_query, DocumentStatus status) const;
    
    vector<Document> FindTopDocuments(const string_view raw_query) const;
    
    template <typename DocumentPredicate>
    
    vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const; 

    int GetDocumentCount() const {
        return documents_.size();
    }

    const set<int>::iterator begin(){
        return document_ids_.begin();
    };
    const set<int>::iterator end(){
        return document_ids_.end();
    };

    tuple<vector<string_view>, DocumentStatus> MatchDocument(const string_view raw_query, const int document_id) const{
        return MatchDocument(execution::seq, raw_query, document_id);
    };
  
    tuple<vector<string_view>, DocumentStatus> MatchDocument(const execution::sequenced_policy&, const string_view raw_query, int document_id) const{
        
        if (documents_.count(document_id) == 0) {
            throw std::out_of_range("Invalid id");
        }
        
        const auto query = ParseQuery(true, raw_query);
        
        vector<string_view> matched_words;
        for (const auto word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                return {vector<string_view> {}, documents_.at(document_id).status};
            }
        }
        
        for (const auto word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
       
        return {matched_words, documents_.at(document_id).status};
    }; 
    
    
    
    tuple<vector<string_view>, DocumentStatus> MatchDocument(const execution::parallel_policy&, const string_view raw_query, int document_id) const{
        
        if (documents_.count(document_id) == 0) {
            throw std::out_of_range("Invalid id");
        }
    
        auto query = ParseQuery(false, raw_query);
        
        auto word_ =[&](const string_view word) {  
           const auto pos = word_to_document_freqs_.find(word);
           return pos != word_to_document_freqs_.end() && pos->second.count(document_id);
    };
        
    if(any_of(execution::par, query.minus_words.begin(), query.minus_words.end(), word_)){
        return {vector<string_view> {}, documents_.at(document_id).status};
    }
        
    vector<string_view> matched_words(query.plus_words.size());
    auto it = copy_if(execution::par, query.plus_words.begin(), query.plus_words.end(),
                  matched_words.begin(), [&](const string_view word) {
                      return word_(word);
                  });
    
    sort(execution::par ,matched_words.begin(), it);
    auto it2 = unique(execution::par ,matched_words.begin(), it);
    matched_words.erase(it2, matched_words.end());
        
   return {matched_words, documents_.at(document_id).status};   
    }
                 

private:
    struct DocumentData {
        int rating;
        string text;
        DocumentStatus status;
    };
    const set<string, less<>> stop_words_;
    map<int, DocumentData> documents_;

    bool IsStopWord(const string_view word) const;

    static bool IsValidWord(const string_view word);

    vector<string_view> SplitIntoWordsNoStop(const string_view text) const;

    static int ComputeAverageRating(const vector<int>& ratings);

    const map<string_view, double>& GetWordFrequencies(int document_id) const;

    QueryWord ParseQueryWord(const string_view text) const;
    
    Query ParseQuery(bool flag, const string_view text) const;

    double ComputeWordInverseDocumentFreq(const string_view word) const;

    template <typename DocumentPredicate, typename ExecutionPolicy>
    vector<Document> FindAllDocuments(ExecutionPolicy&& policy, const Query& query, DocumentPredicate document_predicate) const;
};

    template <typename DocumentPredicate>
    vector<Document> SearchServer::FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const {
        map<int, double> document_to_relevance;
      
        for (auto word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }
        
        for (auto word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
}



template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words))
{  
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw invalid_argument("Some of stop words are invalid"s);
    }
}



template <typename DocumentPredicate>
vector<Document> SearchServer::FindTopDocuments(const string_view raw_query, DocumentPredicate document_predicate) const {

    const auto query = ParseQuery(true,raw_query);
    
    auto matched_documents = FindAllDocuments(query, document_predicate);
    
    sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
        if (abs(lhs.relevance - rhs.relevance) < EPS) {
            return lhs.rating > rhs.rating;
        } else {
            return lhs.relevance > rhs.relevance;
        }
    });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

template <typename ExecutionPolicy>
vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

template <typename ExecutionPolicy>
vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query) const {
    return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <typename DocumentPredicate, typename ExecutionPolicy>
vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const string_view raw_query, DocumentPredicate document_predicate) const {

    //int t1 = clock();
    const auto query = ParseQuery(true,raw_query);
    // int t2 = clock();
    //cout << "PQ " << (double)(t2-t1)/CLOCKS_PER_SEC << endl;
    
    //int t1 = clock();
    auto matched_documents = FindAllDocuments(policy, query, document_predicate);
    //int t2 = clock();
    //cout << "FAD " << (double)(t2-t1)/CLOCKS_PER_SEC << endl;
    
    sort(policy, matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
        if (abs(lhs.relevance - rhs.relevance) < EPS) {
            return lhs.rating > rhs.rating;
        } else {
            return lhs.relevance > rhs.relevance;
        }
    });
    
    
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    
    return matched_documents;
}


template <typename DocumentPredicate, typename ExecutionPolicy>
    vector<Document> SearchServer::FindAllDocuments(ExecutionPolicy&& policy, const Query& query, DocumentPredicate document_predicate) const {
        ConcurrentMap<int, double> document_to_relevance(150);
        //map<int, double> document_to_relevance;
        
        for_each(policy, query.plus_words.begin(), query.plus_words.end(),[&](auto word){
            if (word_to_document_freqs_.count(word) != 0) {
                const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);

                for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                    const auto document_data = documents_.at(document_id);
                    if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    
                        document_to_relevance[document_id].ref_to_value += term_freq * inverse_document_freq;
                        /*
                        
                        document_to_relevance[document_id] = document_to_relevance[document_id]+ term_freq * inverse_document_freq;
                        */

                    }
                }
            }
        });
        
        for_each(policy, query.minus_words.begin(), query.minus_words.end(),[&](auto word){
            if (word_to_document_freqs_.count(word) != 0) {
                for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                    document_to_relevance.erase(document_id);
                }
            }
        });
        
        vector<Document> matched_documents{};
        map<int, double> document_to_relevance_ = document_to_relevance.BuildOrdinaryMap();
        for (const auto [document_id, relevance] : document_to_relevance_) {
            matched_documents.push_back({document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
}


