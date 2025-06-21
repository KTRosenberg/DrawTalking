//
//  regular_expr.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/11/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef regular_expr_hpp
#define regular_expr_hpp

namespace mtt {

mtt::String regex_replace_new_str(const mtt::String& str, const mtt::String& pattern, cstring fmt);

struct Regex_Record {
    void* regex;
    mtt::String label;
};

Regex_Record regex_make(const mtt::String& str);
void regex_destroy(Regex_Record* r);

mtt::String regex_replace_new_str(const mtt::String& str, Regex_Record* record, cstring fmt);

void replace_patterns_in_place(mtt::String& input, std::vector<robin_hood::pair<Regex_Record, mtt::String>>& replacements);
}

#endif /* regular_expr_hpp */
