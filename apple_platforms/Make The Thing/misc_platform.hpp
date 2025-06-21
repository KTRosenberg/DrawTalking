//
//  misc_platform.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/1/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef misc_platform_h
#define misc_platform_h

#include "misc.hpp"

void subscribe_message_platform(const mtt::String& message_name, void (*handler)(void*, void*), void* ctx);


#endif /* misc_platform_h */
