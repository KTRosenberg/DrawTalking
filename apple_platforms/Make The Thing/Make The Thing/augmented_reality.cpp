//
//  augmented_reality.cpp
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/30/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "augmented_reality_platform.hpp"

namespace mtt {

bool Augmented_Reality_load(Augmented_Reality_Context* ctx)
{
    return Augmented_Reality_load_platform(ctx);
}

void Augmented_Reality_unload(Augmented_Reality_Context* ctx)
{
    return Augmented_Reality_unload_platform(ctx);
}

bool Augmented_Reality_run(Augmented_Reality_Context* ctx)
{
    return Augmented_Reality_run_platform(ctx);
}

void Augmented_Reality_pause(Augmented_Reality_Context* ctx)
{
    Augmented_Reality_pause_platform(ctx);
}

void Augmented_Reality_config(Augmented_Reality_Context* ctx, uint64 flags)
{
    Augmented_Reality_config_platform(ctx, flags);
}

}
