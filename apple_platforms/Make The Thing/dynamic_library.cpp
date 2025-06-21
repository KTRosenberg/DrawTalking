//
//  dynamic_library.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/1/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#include "dynamic_library.hpp"

void* MTT_Dynamic_Library::operator()(const std::string& name, void* args)
{
    return MTT_Dynamic_Library_call_proc(this, name, args);
}
