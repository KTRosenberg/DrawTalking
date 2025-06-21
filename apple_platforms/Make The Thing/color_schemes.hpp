//
//  color_schemes.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/25/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#ifndef color_schemes_hpp
#define color_schemes_hpp

#include "thing_shared_types.hpp"

namespace mtt {

typedef int Color_Scheme_ID;

Color_Scheme_ID color_scheme(void);
void set_color_scheme(Color_Scheme_ID color_scheme_id);

inline static const constexpr Color_Scheme_ID COLOR_SCHEME_DARK_MODE = 0;
inline static const constexpr Color_Scheme_ID COLOR_SCHEME_LIGHT_MODE = 1;

inline static cstring COLOR_SCHEME_STRINGS[] = {
    [COLOR_SCHEME_DARK_MODE]  = "COLOR_SCHEME_DARK_MODE",
    [COLOR_SCHEME_LIGHT_MODE] = "COLOR_SCHEME_LIGHT_MODE",
};

inline static const mtt::Map<mtt::String, Color_Scheme_ID> color_scheme_strings_to_id = {
    {"COLOR_SCHEME_DARK_MODE",  COLOR_SCHEME_DARK_MODE},
    {"DARK_MODE",               COLOR_SCHEME_DARK_MODE},
    {"DM",                      COLOR_SCHEME_DARK_MODE},
    
    {"COLOR_SCHEME_LIGHT_MODE", COLOR_SCHEME_LIGHT_MODE},
    {"LIGHT_MODE",              COLOR_SCHEME_LIGHT_MODE},
    {"LM",                      COLOR_SCHEME_LIGHT_MODE}
};


}

#endif /* color_schemes_hpp */
