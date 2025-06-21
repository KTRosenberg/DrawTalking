//
//  color_schemes.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/25/22.
//  Copyright © 2022 Toby Rosenberg. All rights reserved.
//

#include "color_schemes.hpp"

namespace mtt {

Color_Scheme_ID COLOR_SCHEME = COLOR_SCHEME_LIGHT_MODE;

void set_color_scheme(Color_Scheme_ID color_scheme_id)
{
    COLOR_SCHEME = color_scheme_id;
}

int color_scheme(void)
{
    return COLOR_SCHEME;
}

}
