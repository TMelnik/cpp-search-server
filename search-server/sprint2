#include "search_server.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

/* Подставьте вашу реализацию класса SearchServer сюда */
/*
string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word)
;
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word)
;
    }

    return words;
}



struct Document {
    int id;
    double relevance;
    int rating;
};


enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};


void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status) {
    cout << "{ "
         << "document_id = " << document_id << ", "
         << "status = " << static_cast<int>(status) << ", "
         << "words =";
    for (const string& word : words) {
        cout << ' ' << word;
    }
    cout << "}" << endl;
}


class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word)
;
        }
    }

    int GetDocumentCount()const {
        return documents_.size();
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id,
            DocumentData{
                ComputeAverageRating(ratings),
                status
            });
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, status);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
            });
        if (matched_documents.size() > 5) {
            matched_documents.resize(5);
        }
        return matched_documents;
    }


    template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const {

        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, document_predicate);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
            });
        if (matched_documents.size() > 5) {
            matched_documents.resize(5);
        }
        return matched_documents;

        // ¬аша реализаци€ данного метода
    }


    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const{
        vector<string> matched_words = {};
        const Query query = ParseQuery(raw_query);

        for (const auto& word : query.plus_words){
            if (word_to_document_freqs_.count(word) == 0){
                continue;
            }
            else if (word_to_document_freqs_.at(word).count(document_id) != 0){
                if (count(matched_words.begin(), matched_words.end(), word) == 0){
                    matched_words.push_back(word);
                }
            }

        }

        for (const auto& word : query.minus_words){
            if (word_to_document_freqs_.count(word)== 0){
                continue;
            }
            else  if (word_to_document_freqs_.at(word).count(document_id) != 0){
                return tuple<vector<string>, DocumentStatus>(vector<string>{}, documents_.at(document_id).status);
            }

        }
        sort(matched_words.begin(), matched_words.end());

        return tuple<vector<string>, DocumentStatus>(matched_words, documents_.at(document_id).status );
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word)
 > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word)
;
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {
            text,
            is_minus,
            IsStopWord(text)
        };
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word)
;
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }


    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(documents_.size() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    vector<Document> FindAllDocuments(const Query& query, DocumentStatus status = DocumentStatus::ACTUAL) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                if (documents_.at(document_id).status == status) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word)== 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({
                document_id,
                relevance,
                documents_.at(document_id).rating
            });
        }
        //for(int i=0; i<)
        return matched_documents;
    }



template <typename DocumentPredicate>
vector<Document> FindAllDocuments(const Query& query, DocumentPredicate predicat) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word)
 == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word)
;
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                if (predicat(document_id, documents_.at(document_id).status, documents_.at(document_id).rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word)
 == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({
                document_id,
                relevance,
                documents_.at(document_id).rating
            });
        }
        return matched_documents;
    }


};
*/

/*
   Подставьте сюда вашу реализацию макросов
   ASSERT, ASSERT_EQUAL, ASSERT_EQUAL_HINT, ASSERT_HINT и RUN_TEST

*/
template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

//#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

//#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint) {
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cout << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

//#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

//#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename T>
void RunTestImpl(const T& t, const string& func_name) {
        t();
        cerr << func_name << " OK"s << endl;
}

//#define RUN_TEST(func) RunTestImpl((func), #func)


// -------- Начало модульных тестов поисковой системы ----------

//Добавление документов. Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
    }
}

/*
Разместите код остальных тестов здесь
*/

//документы, содержащие минус-слова поискового запроса, не должны включатьс€ в результаты поиска.
void TestExcludeMinusWordsFromAddedDocumentContent(){
    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {1, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "cat and dog in the home";

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("cat -city");
        ASSERT_EQUAL(found_docs.size(), 1);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id2);
    }

}

//Соответствие документов поисковому запросу. При этом должны быть возвращены все слова из поискового запроса, присутствующие в документе. Если есть соответствие хотя бы по одному минус-слову, должен возвращаться пустой список слов.
void TestFindDocumentWithSearchWords(){
    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {1, 2, 3};

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.MatchDocument("cat -city", doc_id);
        ASSERT(get<0>(found_docs).size() !=1);

        const auto found_docs2 = server.MatchDocument("cat -dog", doc_id);
        ASSERT_EQUAL(get<0>(found_docs).size(), 1);
    }

}

//  Сортировка найденных документов по релевантности. Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
void TestSortRating(){
    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {1, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "cat and dog in the home";

    const int doc_id3 = 44;
    const string content3 = "in the box";

     {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("cat dog");
        ASSERT_EQUAL(found_docs.size(), 2);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id2);
        const Document& doc1 = found_docs[1];
        ASSERT_EQUAL(doc1.id, doc_id);
     }
}

//  Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему арифметическому оценок документа.
void TestComputeAverageRating(){
    //static int ComputeAverageRating(const vector<int>& ratings)
    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {1, 2, 3};
    {
       SearchServer server;
       server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);

       const auto found_docs = server.FindTopDocuments("cat dog");
       ASSERT_EQUAL(found_docs[0].rating, 2);
    }
}

//Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestPredicate(){

    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {1, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "cat and dog in the home";
    const vector<int> ratings2 = {3, 3, 3};

    const int doc_id3 = 44;
    const string content3 = "cat in the box";
    const vector<int> ratings3 = {4, 4, 4};

    {
       SearchServer server;
       server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
       server.AddDocument(doc_id2, content2, DocumentStatus::IRRELEVANT, ratings);
       server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings);

       const auto found_docs = server.FindTopDocuments("cat in box", [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; });

        ASSERT_EQUAL(found_docs[0].id, 44);
        ASSERT_EQUAL(found_docs[1].id, 42);
    }
}

//Поиск документов, имеющих заданный статус.
void TestStatus(){

    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {1, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "cat and dog in the home";
    const vector<int> ratings2 = {3, 3, 3};

    const int doc_id3 = 44;
    const string content3 = "cat and dog in the black box";
    const vector<int> ratings3 = {4, 4, 4};

    {
       SearchServer server;
       server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
       server.AddDocument(doc_id2, content2, DocumentStatus::IRRELEVANT, ratings);
       server.AddDocument(doc_id3, content3, DocumentStatus::REMOVED, ratings);
       const auto found_docs = server.FindTopDocuments("cat", DocumentStatus::IRRELEVANT);
       ASSERT_EQUAL(found_docs.size(), 1);
       ASSERT_EQUAL(found_docs[0].id, 43);
       const auto found_docs2 = server.FindTopDocuments("cat", DocumentStatus::REMOVED);
       ASSERT_EQUAL(found_docs2.size(), 1);
       ASSERT_EQUAL(found_docs2[0].id, 44);

    }


}

//  Корректное вычисление релевантности найденных документов.

void TestCountingRelevansIsCorrect(){

    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {1, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "cat and dog in the home";
    const vector<int> ratings2 = {3, 3, 3};

    const int doc_id3 = 44;
    const string content3 = "cat and dog in the black box";
    const vector<int> ratings3 = {4, 4, 4};

    {
       SearchServer server;
       server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
       server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings);
       server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings);
       const auto found_docs = server.FindTopDocuments("cat dog -city", DocumentStatus::ACTUAL);
       ASSERT_EQUAL(found_docs.size(), 2);
       ASSERT_EQUAL(found_docs[0].id, 43);
       ASSERT_EQUAL(found_docs[1].id, 44);
    }
}



/* ‘ункци€ TestSearchServer €вл€етс€ точкой входа дл€ запуска тестов
void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();
    TestExcludeMinusWordsFromAddedDocumentContent();
    TestSortRating();
    TestComputeAverageRating();
    TestPredicate();
    TestStatus();
    TestCountingRelevansIsCorrect();

    // Ќе забудьте вызывать остальные тесты здесь
}
*/

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeMinusWordsFromAddedDocumentContent);
    RUN_TEST(TestSortRating);
    RUN_TEST(TestComputeAverageRating);
    RUN_TEST(TestPredicate);
    RUN_TEST(TestStatus);
    RUN_TEST(TestCountingRelevansIsCorrect);
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}
