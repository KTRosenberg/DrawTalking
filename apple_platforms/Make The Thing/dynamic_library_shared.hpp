//
//  dynamic_library_shared.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/1/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef dynamic_library_shared_hpp
#define dynamic_library_shared_hpp

#include "stratadraw.h"
#include "dynamic_api.hpp"

namespace mtt {

struct DL_Ctx {
    MTT_Core* core;
//    float64 time_seconds;
//    uint64 time_nanoseconds;
//    sd::Renderer* renderer;
//    sd::Viewport* viewport;
    MTT_API mtt_api;
    sd::Renderer_API sd_api;
    
    mtt::Map<mtt::String, mtt::Any> vars;
    void* state = nullptr;
    void* params = nullptr;
};


}

#endif /* dynamic_library_shared_hpp */
