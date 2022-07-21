#pragma once

#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Document {
    Document() = default;

    Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {
    }

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};


enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

ostream& operator<<(ostream& out, const Document& document);


void PrintDocument(const Document& document);

void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status);
/*
void AddDocument(SearchServer& search_server, int document_id, const string& document, DocumentStatus status,const vector<int>& ratings);


void FindTopDocuments(const SearchServer& search_server, const string& raw_query);

void MatchDocuments(const SearchServer& search_server, const string& query);
*/

