
#include <algorithm>
#include <cstdlib>
#include <future>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <vector>
#include <mutex>
 
#include "log_duration.h"
#include "test_framework.h"
#include "search_server.h"
#include "concurrent_map.h"
 
using namespace std::string_literals;
using namespace std;

 
void RunConcurrentUpdates(ConcurrentMap<int, int>& cm, size_t thread_count, int key_count) {
    auto kernel = [&cm, key_count](int seed) {
        vector<int> updates(key_count);
        iota(begin(updates), end(updates), -key_count / 2);
        shuffle(begin(updates), end(updates), mt19937(seed));
 
        for (int i = 0; i < 2; ++i) {
            for (auto key : updates) {
                ++cm[key].ref_to_value;
            }
        }
    };
 
    vector<future<void>> futures;
    for (size_t i = 0; i < thread_count; ++i) {
        futures.push_back(async(kernel, i));
    }
}

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries){
    
    std::vector<std::vector<Document>> v(queries.size());
    
    transform(execution::par,
        queries.begin(), queries.end(),
        v.begin(),
        [&search_server](const string& str) { return search_server.FindTopDocuments(str); }
    );
    return v;
} 

vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries){
    
    return transform_reduce(execution::par,
        queries.begin(), queries.end(),
        vector<Document>{},
        [](vector<Document> T1, vector<Document> const& T2) {
            T1.insert(T1.end(), T2.begin(), T2.end());
            return T1;
        },             
        [&search_server](const string& str) { return search_server.FindTopDocuments(str); }
    );
    
}

void TestConcurrentUpdate() {
    constexpr size_t THREAD_COUNT = 3;
    constexpr size_t KEY_COUNT = 50000;
 
    ConcurrentMap<int, int> cm(THREAD_COUNT);
    RunConcurrentUpdates(cm, THREAD_COUNT, KEY_COUNT);
 
    const auto result = cm.BuildOrdinaryMap();
    ASSERT_EQUAL(result.size(), KEY_COUNT);
    for (auto& [k, v] : result) {
        AssertEqual(v, 6, "Key = " + to_string(k));
    }
}
 
void TestReadAndWrite() {
    ConcurrentMap<size_t, string> cm(5);
 
    auto updater = [&cm] {
        for (size_t i = 0; i < 50000; ++i) {
            cm[i].ref_to_value.push_back('a');
        }
    };
    auto reader = [&cm] {
        vector<string> result(50000);
        for (size_t i = 0; i < result.size(); ++i) {
            result[i] = cm[i].ref_to_value;
        }
        return result;
    };
 
    auto u1 = async(updater);
    auto r1 = async(reader);
    auto u2 = async(updater);
    auto r2 = async(reader);
 
    u1.get();
    u2.get();
 
    for (auto f : {&r1, &r2}) {
        auto result = f->get();
        ASSERT(all_of(result.begin(), result.end(), [](const string& s) {
            return s.empty() || s == "a" || s == "aa";
        }));
    }
}
 
void TestSpeedup() {
    {
        ConcurrentMap<int, int> single_lock(1);
 
        LOG_DURATION("Single lock");
        RunConcurrentUpdates(single_lock, 4, 50000);
    }
    {
        ConcurrentMap<int, int> many_locks(100);
 
        LOG_DURATION("100 locks");
        RunConcurrentUpdates(many_locks, 4, 50000);
    }
}
/*
int main() {
    TestRunner tr;
    RUN_TEST(tr, TestConcurrentUpdate);
    RUN_TEST(tr, TestReadAndWrite);
    RUN_TEST(tr, TestSpeedup);
}
*/


#include "process_queries.h"
#include "search_server.h"
#include <execution>
#include <iostream>
#include <string>
#include <vector>
using namespace std;


int main() {
    SearchServer search_server("and with"s);
    int id = 0;
    for (
        const string& text : {
            "white cat and yellow hat"s,
            "curly cat curly tail"s,
            "nasty dog with big eyes"s,
            "nasty pigeon john"s,
        }
    ) {
        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
    }
    cout << "ACTUAL by default:"s << endl;
    // последовательная версия
    for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
        PrintDocument(document);
    }
    cout << "BANNED:"s << endl;
    // последовательная версия
    for (const Document& document : search_server.FindTopDocuments(execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }
    cout << "Even ids:"s << endl;
    // параллельная версия
    for (const Document& document : search_server.FindTopDocuments(execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
        PrintDocument(document);
    }
    return 0;
} 



/*
#include "search_server.h"
#include "log_duration.h"
#include <execution>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;
string GenerateWord(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution('a', 'z')(generator));
    }
    return word;
}
vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}
string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0) {
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
            query.push_back('-');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}
vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}
template <typename ExecutionPolicy>
void Test(string_view mark, const SearchServer& search_server, const vector<string>& queries, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    double total_relevance = 0;
    for (const string_view query : queries) {
        for (const auto& document : search_server.FindTopDocuments(policy, query)) {
            total_relevance += document.relevance;
        }
    }
    cout << total_relevance << endl;
}

#define TEST(policy) Test(#policy, search_server, queries, execution::policy)

int main() {
    mt19937 generator;
    const auto dictionary = GenerateDictionary(generator, 1000, 10);
    const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);
    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) {
        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
    }
    const auto queries = GenerateQueries(generator, dictionary, 100, 70);
    TEST(seq);
    TEST(par);
} 
*/
