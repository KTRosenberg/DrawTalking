//
//  dynamic_main.hpp
//  dynamic_dev
//
//  Created by Toby Rosenberg on 4/1/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef dynamic_main_hpp
#define dynamic_main_hpp

#include "dynamic_library.hpp"

#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
#define MTT_DYNAMIC_EXPORT __attribute__((visibility("default")))
#else
#define MTT_DYNAMIC_EXPORT


namespace ext {
void* on_before_frame(void* args);
void* on_frame(void* args);
void* on_after_frame(void* args);
void* setup(void* args);
void* on_resize(void* args);
void* on_unload(void* args);
}

#endif


#include "dynamic_library_shared.hpp"


#endif /* dynamic_main_hpp */
