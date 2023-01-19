 #pragma once

#include <set>
#include <string>
#include <vector>
#include <functional>
#include <iostream>

template <typename StringContainer>
std::set<std::string, std::less<>> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string, std::less<>> non_empty_strings;
    for (auto &str : strings) {
        if (!str.empty()) {
            non_empty_strings.emplace(str);
        }
    }
    
    return non_empty_strings;
}
std::vector<std::string_view> SplitIntoWords(const std::string_view text);
