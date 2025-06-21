//
//  string_intern.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 9/28/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "string_intern.hpp"

MTT_String_Ref::operator cstring()
{
    return MTT_string_ref_to_cstring_checked(*this);
}

static_assert(std::is_standard_layout<MTT_String_Ref>::value && std::is_trivially_copyable<MTT_String_Ref>::value, "check that TT_String_Ref has standard layout and is trivially copiable");

namespace mtt {

}
