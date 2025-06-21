//
//  augmented_reality_platform.hpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/30/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef augmented_reality_platform_hpp
#define augmented_reality_platform_hpp

#include "augmented_reality.hpp"


namespace mtt {

bool Augmented_Reality_load_platform(Augmented_Reality_Context* ctx);
void Augmented_Reality_unload_platform(Augmented_Reality_Context* ctx);

void Augmented_Reality_init_platform(Augmented_Reality_Context* ctx, void* host);
    
bool Augmented_Reality_run_platform(Augmented_Reality_Context* ctx);

void Augmented_Reality_pause_platform(Augmented_Reality_Context* ctx);

void Augmented_Reality_config_platform(Augmented_Reality_Context* ctx, uint64 flags);

inline void Augmented_Reality_set_active_state(Augmented_Reality_Context* ctx, bool state)
{
    ctx->is_active = state;
}

}


#endif /* augmented_reality_platform_hpp */
