#include "search_server.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

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

//документы, содержащие минус-слова поискового запроса, не должны включаться в результаты поиска.
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
        ASSERT_EQUAL(found_docs.size(), 1u);
        ASSERT_EQUAL(found_docs[0].id, doc_id2);
    }

}

//Соответствие документов поисковому запросу. При этом должны быть возвращены все слова из поискового запроса, присутствующие в документе. Если есть соответствие хотя бы по одному минус-слову, должен возвращаться пустой список слов.
void TestFindDocumentWithSearchWords(){
    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {1, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "cat in the box";
    const vector<int> ratings2 = {1, 3, 4};

    {
        SearchServer server;
        server.SetStopWords("in the box"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const found_docs= server.MatchDocument("cat -city", doc_id);
        const auto& [x, y] = found_docs;
        ASSERT_EQUAL(x.size(), 0u);

        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found_docs2 = server.MatchDocument("cat -dog", doc_id);
        const auto& [x, y] = found_docs2;
        ASSERT_EQUAL(x.size(), 1u);
        ASSERT_EQUAL(x[0], "cat");
        ASSERT_EQUAL(y, ACTUAL);

        const auto found_docs3 = server.MatchDocument("cat in the box", doc_id2);
        const auto& [x, y] = found_docs3;
        ASSERT_EQUAL(x.size(), 1u);
        ASSERT_EQUAL(x[0], "cat");
    }

}

//  Сортировка найденных документов по релевантности. Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
void TestSortRalevance(){
    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {1, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "cat and dog in the home";

    const int doc_id3 = 44;
    const string content3 = "and dog in the box";

     {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id3, content3, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("cat and dog");
        ASSERT_EQUAL(found_docs.size(), 3u);
        ASSERT_EQUAL(found_docs[0].id, doc_id2);
        ASSERT_EQUAL(found_docs[1].id, doc_id3);
        ASSERT_EQUAL(found_docs[2].id, doc_id);
     }
}

//  Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему арифметическому оценок документа.
void TestComputeAverageRating(){

    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {2, 2, 3};

    const int doc_id2 = 43;
    const string content2 = "dog in the box";
    const vector<int> ratings2 = {2, 2, 3, 3, 3};

    {
       SearchServer server;
       server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
       const auto found_docs = server.FindTopDocuments("cat");
       ASSERT_EQUAL(found_docs[0].rating, 2.33333);

       server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
       const auto found_docs = server.FindTopDocuments("dog");
       ASSERT_EQUAL(found_docs[0].rating, 2.666667);
    }
}

//Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestCheckPredicate(){

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
       const auto found_docs = server.FindTopDocuments("cat in box",
              [](int document_id, DocumentStatus status, int rating)
              { return status == DocumentStatus::ACTUAL; });
       ASSERT_EQUAL(found_docs[0].id, 44);
       ASSERT_EQUAL(found_docs[1].id, 42);
    }
}

//Поиск документов, имеющих заданный статус.
void TestCheckStatus(){

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
       ASSERT_EQUAL(found_docs.size(), 1u);
       ASSERT_EQUAL(found_docs[0].id, 43);
       const auto found_docs2 = server.FindTopDocuments("cat", DocumentStatus::REMOVED);
       ASSERT_EQUAL(found_docs2.size(), 1u);
       ASSERT_EQUAL(found_docs2[0].id, 44);
       const auto found_docs2 = server.FindTopDocuments("cat", DocumentStatus::BANNED);
       ASSERT_EQUAL(found_docs2.size(), 0u);
       
    }
}

//  Корректное вычисление релевантности найденных документов.

void TestCountRelevans(){

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
       ASSERT_EQUAL(found_docs.size(), 2u);
       ASSERT_EQUAL(found_docs[0].id, 43);
       ASSERT_EQUAL(found_docs[1].id, 44);
    }
}


// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeMinusWordsFromAddedDocumentContent);
    RUN_TEST(TestSortRating);
    RUN_TEST(TestComputeAverageRating);
    RUN_TEST(TestPredicate);
    RUN_TEST(TestStatus);
    RUN_TEST(TestCountingRelevansIsCorrect);
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}
