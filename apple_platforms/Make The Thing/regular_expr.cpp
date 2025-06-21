//
//  regular_expr.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/11/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#include "regular_expr.hpp"

#include <regex>

namespace mtt {

using BASIC_REGEX = std::basic_regex<char>;

Regex_Record regex_make(const mtt::String& str)
{
    return (Regex_Record){.regex = (void*)(new BASIC_REGEX(str)), .label = str};
}

void regex_destroy(Regex_Record* r)
{
    if (r->regex != nullptr) {
        delete ((BASIC_REGEX*)r->regex);
        r->regex = nullptr;
    }
}

mtt::String regex_replace_new_str(const mtt::String& str, const mtt::String& pattern, cstring fmt)
{
    return std::regex_replace(str, BASIC_REGEX(pattern), fmt);
}

mtt::String regex_replace_new_str(const mtt::String& str, Regex_Record* record, cstring fmt)
{
    BASIC_REGEX& regex = *((BASIC_REGEX*)record->regex);
    return std::regex_replace(str, regex, fmt);
}

void replace_patterns_in_place(mtt::String& input, std::vector<robin_hood::pair<Regex_Record, mtt::String>>& replacements)
{
    for (auto it = replacements.begin(); it != replacements.end(); ++it) {
        mtt::Regex_Record* record = &it->first;
        input = std::regex_replace(input, *((BASIC_REGEX*)record->regex), it->second);
    }
}

}
