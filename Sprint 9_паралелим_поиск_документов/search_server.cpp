#include "search_server.h"

#include <cmath>
#include <numeric>
#include <utility>
#include <functional>

using namespace std;

vector<Document> SearchServer::FindTopDocuments(const string_view raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

bool SearchServer::IsStopWord(const string_view word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const string_view word) {

    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = 0;

    return accumulate(ratings.begin(), ratings.end(), rating_sum) / static_cast<int>(ratings.size());
}

const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const{
    static map<string_view, double> id_frequencies;
    
    if(document_ids_.count(document_id)){
       
        auto it = word_to_document_freqs_.begin();
        for(; it != word_to_document_freqs_.end(); ++it){
            auto it2 = it->second.find(document_id);
            if(it2 != it->second.end()){
               id_frequencies.at(it->first) =  it2->second;
            }
        }
    }
    
    return id_frequencies;
}


void SearchServer::RemoveDocument(int document_id){
	if(!count(document_ids_.begin(), document_ids_.end(), document_id)){
            return;
        }    
	auto it = document_words_freqs_.find(document_id);
        for(auto it2: it->second){
            word_to_document_freqs_.find(it2.first)->second.erase(document_id);
        }
    
        documents_.erase(document_id);
        document_ids_.erase(find(document_ids_.begin(), document_ids_.end(), document_id));
}

void SearchServer::AddDocument(int document_id, const string_view document, DocumentStatus status, const vector<int>& ratings) {
    
        if ((document_id < 0) || (documents_.count(document_id) > 0)) {
            throw invalid_argument("Invalid document_id"s);
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), string(move(document)), status});
    
        const vector<string_view> words = SplitIntoWordsNoStop(documents_[document_id].text);
    
        const double inv_word_count = 1.0 / words.size();
       
        for (auto word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
            document_words_freqs_[document_id][word]+=inv_word_count;
        }
    
        document_ids_.insert(document_id);
}



vector<Document> SearchServer::FindTopDocuments( const string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}


vector<string_view> SearchServer::SplitIntoWordsNoStop(const string_view text) const {
        vector<string_view> words;
           
        for (auto word : SplitIntoWords(text)) {
            if (!IsValidWord(word)) {
                throw invalid_argument("Word "s + string(word) + " is invalid"s);
            }
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
}


SearchServer::QueryWord SearchServer::ParseQueryWord(const string_view text) const {
        if (text.empty()) {
            throw invalid_argument("Query word is empty"s);
        }
        string_view word = text;
        bool is_minus = false;
        if (word[0] == '-') {
            is_minus = true;
            word = word.substr(1);
        }
        if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
            throw invalid_argument("Query word "s + (string)text + " is invalid");
        }

        return {word, is_minus, IsStopWord(word)};
}

SearchServer::Query SearchServer::ParseQuery(bool flag, const string_view text) const {
    
    SearchServer::Query result;
    vector<string_view> v = SplitIntoWords(text);
     
    for(auto word : v){
        const auto query_word = ParseQueryWord(word);
        
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(query_word.data);
            } else {
                result.plus_words.push_back(query_word.data);
            }
        }
    } 
    
    if(flag){
        sort(result.minus_words.begin(), result.minus_words.end());
        auto it2 = unique(result.minus_words.begin(), result.minus_words.end());
        result.minus_words.erase(it2, result.minus_words.end());
        
        sort(result.plus_words.begin(), result.plus_words.end());
        auto it3 = unique(result.plus_words.begin(), result.plus_words.end());
        result.plus_words.erase(it3, result.plus_words.end());
    }
        return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string_view word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
