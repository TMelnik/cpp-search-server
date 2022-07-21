#include "request_queue.h"

using namespace std;


bool RequestQueue::IsFullQueue(){
        if(tail == 1440 || tail == head -1){
            return true;
        } else
            return false;
}

RequestQueue::RequestQueue(const SearchServer& search_server) {
        search_server_= &search_server;
        requests_.resize(min_in_day_ + 1);
}

void RequestQueue::UpdateQueue(){

        if(v.size() > 0){

            requests_[tail].cell = 1;
            --count;

        }else {
            requests_[tail].cell = 0;
            ++count;
        }

        if(IsFullQueue()){
                if(head == 1440)
                    head = 0;
                else
                    ++head;
                if(tail == 1440)
                    tail = 0;
                else
                    ++tail;
          }
    }

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {

        v= search_server_->FindTopDocuments(raw_query,status);
        UpdateQueue();

        return v;
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {

        v= search_server_->FindTopDocuments(raw_query);
        UpdateQueue();

        return v;
}
