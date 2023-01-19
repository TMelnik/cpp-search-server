#include "string_processing.h"

using namespace std;

vector<string_view> SplitIntoWords(const string_view text) {
    vector<string_view> words;
    string_view word;
    int i = 0, len = 0;
    for (const char &c : text) {
        if (c == ' ') {
            word = text.substr(i-len, len);
            len=0;
            if (!word.empty()) {
                words.push_back(word);
            }
        } else {
            len++;
        }
        i++;
    }
    word = text.substr(i-len, len);
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}
