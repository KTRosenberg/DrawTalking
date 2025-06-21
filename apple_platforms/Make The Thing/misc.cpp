//
//  misc.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/1/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#include "misc_platform.hpp"


void subscribe_message(const mtt::String& message_name, void (*handler)(void*, void*), void* ctx)
{
    subscribe_message_platform(message_name, handler, ctx);
}
