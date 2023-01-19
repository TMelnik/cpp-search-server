#pragma once

#include <string>
#include <vector>
#include <deque>
#include "search_server.h"

using namespace std;


class RequestQueue {
public:

    bool IsFullQueue();

    explicit RequestQueue(const SearchServer& search_server);

    void UpdateQueue();

    template <typename DocumentPredicate>
    vector<Document> AddFindRequest(const string& raw_query, DocumentPredicate document_predicate) {

        v= search_server_->FindTopDocuments(raw_query,document_predicate);
        UpdateQueue();

        return v;

    }

    vector<Document> AddFindRequest(const string& raw_query, DocumentStatus status);

    vector<Document> AddFindRequest(const string& raw_query);

    int GetNoResultRequests() const {
        return count;
    }

private:
    int head = 0;
    int tail = 0;

    struct QueryResult {
        bool cell;
    };

    deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    vector<Document> v;
    int count = 1;
    const SearchServer *search_server_;
};
