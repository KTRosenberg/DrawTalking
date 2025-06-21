//
//  augmented_reality_platform.mm
//  Make The Thing
//
//  Created by Karl Rosenberg on 6/30/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//


#if !MTT_PLATFORM_MACOS
#include "augmented_reality_platform.hpp"
#endif

#include "MTTViewController.hpp"


#include <TargetConditionals.h>

namespace mtt {
    


void Augmented_Reality_init_platform(Augmented_Reality_Context* ctx, void* host)
{
    ctx->is_loaded = false;
    ctx->is_active = false;
    ctx->flags = 0;
    ctx->is_supported = false;
    ctx->backend_ctx = nullptr;
    ctx->backend_host = host;
    ctx->flags = AUGMENTED_REALITY_FLAG_FACE_TRACKING;
    ctx->freeze_frame_when_off = false;
}
    
bool Augmented_Reality_load_platform(Augmented_Reality_Context* ctx)
    {
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    return false;
#elif (USE_ARKIT)
    MTTViewController* host = (__bridge MTTViewController*)ctx->backend_host;

    BOOL status = [host ARKit_load];
    if (status == NO) {
        return false;
    }
    
    ARSession* session = [host ARSession_get];
    if (session != nil) {
        ctx->is_supported = true;
        ctx->backend_host = (__bridge void*)host;
        ctx->is_loaded = true;
    } else {
        MTT_error("%s", "ERROR: could not load AR session\n");
        return false;
    }
    
    return true;
#else
    return false;
#endif

}
    
void Augmented_Reality_unload_platform(Augmented_Reality_Context* ctx)
    {
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    return;
#else
    if (!ctx->is_supported || !ctx->is_loaded) {
        return;
    }
        
    MTTViewController* host = (__bridge MTTViewController*)ctx->backend_host;

    [host ARKit_unload];

    ctx->backend_ctx = nil;
    ctx->is_active   = false;
    ctx->is_loaded   = false;
    ctx->flags = false;
    ctx->backend_host = nil;
#endif
}
    
bool Augmented_Reality_run_platform(Augmented_Reality_Context* ctx)
{
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    return false;
#elif (USE_ARKIT)
    if (!ctx->is_supported || !ctx->is_loaded) {
        return false;
    }
    
    ARSession* ar_session = [((__bridge MTTViewController*)ctx->backend_host) ARSession_get];
    
    if (ctx->flags & AUGMENTED_REALITY_FLAG_FACE_TRACKING) {
        if ([ARFaceTrackingConfiguration isSupported]) {
            [ar_session runWithConfiguration:[[ARFaceTrackingConfiguration alloc] init]];
        } else {
            return false;
        }
    } else if (ctx->flags & AUGMENTED_REALITY_FLAG_WORLD_TRACKING) {
        if ([ARWorldTrackingConfiguration isSupported]) {
            [ar_session runWithConfiguration:[[ARWorldTrackingConfiguration alloc] init]];
        } else {
            return false;
        }
    } else {
        return false;
    }
    
    mtt::Augmented_Reality_set_active_state(ctx, true);
    return true;
#endif
    return false;
}

void Augmented_Reality_pause_platform(Augmented_Reality_Context* ctx)
{
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    return;
#elif (USE_ARKIT)
    if (!ctx->is_supported || !ctx->is_loaded) {
        return;
    }
    
    if (!ctx->is_active) {
        return;
    }
    
    ARSession* ar_session = (__bridge ARSession*)ctx->backend_ctx;
    [ar_session pause];
    mtt::Augmented_Reality_set_active_state(ctx, false);
#endif
}
    
void Augmented_Reality_config_platform(Augmented_Reality_Context* ctx, uint64 flags)
{
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
    return;
#else
    ctx->flags = flags;
#endif
}
    

    

    
}
